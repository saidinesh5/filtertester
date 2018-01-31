#ifndef EMA_NOISE_FILTER_H
#define EMA_NOISE_FILTER_H

#include <math.h>

//Thanks a LOT to http://damienclarke.me/code/posts/writing-a-better-noise-reducing-analogread

template<class T>
class EMANoiseFilter
{
  public:
    EMANoiseFilter(T lowerBound,
                   T upperBound,
                   bool sleepEnabled = true,
                   double snapMultiplier = 0.01):
        mEnabled(upperBound > lowerBound),
        mFirstValue(true),
        mSleepEnabled(sleepEnabled),
        mSleeping(false),
        mErrorEMA(0),
        mLowerBound(lowerBound),
        mUpperBound(upperBound),
        mSnapMultiplier(snapMultiplier),
        mActivityThreshold((upperBound - lowerBound)*0.01) //Activity threshold is 1%
    {
    }

    /**
     * @brief value
     * @return returns the last filtered value after calling update()
     */
    T value() const
    {
        return mFilteredValue;
    }

    /**
     * @brief rawValue
     * @return retuns the last raw value sent into update()
     */
    T rawValue() const
    {
        return mRawValue;
    }

    T lowerBound() const
    {
        return mLowerBound;
    }

    void setLowerBound(T lowerBound)
    {
        mLowerBound = lowerBound;
    }

    T upperBound() const
    {
        return mUpperBound;
    }

    void setUpperBound(T upperBound)
    {
        mUpperBound = upperBound;
    }

    /**
     * @brief hasChanged
     * @return returns if the value has changed during the last update()
     */
    bool hasChanged() const
    {
        return mFilteredValueHasChanged;
    }

    /**
     * @brief isEnabled
     * @return true when the filter is enabled, and the update() actually smoothes the input values
     */
    bool isEnabled() const
    {
        return mEnabled;
    }

    /**
     * @brief setEnabled
     * @param enabled - true when the filter is enabled
     */
    void setEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

    /**
     * @brief isSleeping
     * @return If the filter hasn't responded to any change during the last update().
     * Happens when sleeping is enabled and the change is below the activity threshold.
     */
    bool isSleeping() const
    {
        return mSleeping;
    }

    /**
     * @brief update
     * @param rawValue updates the filtered value based on the raw value being sent in
     * @return returns the filtered value
     */
    T update(T rawValue)
    {
        mPrevResponsiveValue = mFilteredValue;
        mFilteredValue = mEnabled
                         ? getFilteredValue(rawValue)
                         : rawValue;

        mFilteredValueHasChanged = mFilteredValue != mPrevResponsiveValue;
        return mFilteredValue;
    }

    /**
     * @brief snapMultiplier
     * @return the snapMultiplier, which specifies how responsive the filtering should be
     */
    double snapMultiplier() const
    {
        return mSnapMultiplier;
    }

    void setSnapMultiplier(double multiplier)
    {
          if(multiplier > 1.0)
          {
              mSnapMultiplier = 1.0;
          }
          else if(multiplier < 0.0)
          {
              mSnapMultiplier = 0.0;
          }
          else
          {
              mSnapMultiplier = multiplier;
          }
    }

    /**
     * @brief sleepEnabled
     * @return true if the value shouldn't change if it is below the activity threshold
     */
    bool sleepEnabled() const
    {
        return mSleepEnabled;
    }

    /**
     * @brief voidSetSleepEnabled
     * @param sleepEnabled
     */
    void setSleepEnabled(bool sleepEnabled)
    {
        mSleepEnabled = sleepEnabled;
    }

    /**
     * @brief edgeSnapEnabled
     * @return true, if edge snap is enabled. i.e when the value is closer to the lower/upper bounds,
     * The value snaps to the lower/upper bounds.
     */
    bool edgeSnapEnabled() const
    {
        return mEdgeSnapEnabled;
    }

    /**
     * @brief setEdgeSnapEnabled
     * @param edgeSnapEnabled when true to enables snapping the value to the lower/upper bounds.
     */
    void setEdgeSnapEnabled(bool edgeSnapEnabled)
    {
        mEdgeSnapEnabled = edgeSnapEnabled;
    }    

    T activityThreshold() const
    {
        return mActivityThreshold;
    }

    void setActivityThreshold(T threshold)
    {
        mActivityThreshold = threshold;
    }


  private:

    T getFilteredValue(T newValue)
    {
        if (mFirstValue)
        {
            mSmoothValue = newValue;
            mFirstValue = false;
        }

        // if sleep and edge snap are enabled and the new value is very close to an edge, drag it a little closer to the edges
        // This'll make it easier to pull the output values right to the extremes without sleeping,
        // and it'll make movements right near the edge appear larger, making it easier to wake up
        if (mSleepEnabled && mEdgeSnapEnabled)
        {
            if (std::abs(newValue - mLowerBound) < mActivityThreshold)
            {
                newValue = mLowerBound + std::abs((std::abs(newValue - mLowerBound)*2 - mActivityThreshold));
            }
            else if( std::abs(newValue - mUpperBound) < mActivityThreshold)
            {
                newValue = mUpperBound - std::abs((std::abs(newValue - mUpperBound)*2 - mActivityThreshold));
            }
        }

        // get difference between new input value and current smooth value
        T diff = std::abs(newValue - mSmoothValue);

        // measure the difference between the new value and current value
        // and use another exponential moving average to work out what
        // the current margin of error is
        mErrorEMA += ((newValue - mSmoothValue) - mErrorEMA) * 0.4;

        // if sleep has been enabled, sleep when the amount of error is below the activity threshold
        if(mSleepEnabled)
        {
          // recalculate sleeping status
          mSleeping = std::abs(mErrorEMA) < mActivityThreshold;
        }

        // Only update the value if we are not sleeping
        if(!(mSleepEnabled && mSleeping))
        {
            // use a 'snap curve' function, where we pass in the diff (x) and get back a number from 0-1.
            // We want small values of x to result in an output close to zero, so when the smooth value is close to the input value
            // it'll smooth out noise aggressively by responding slowly to sudden changes.
            // We want a small increase in x to result in a much higher output value, so medium and large movements are snappy and responsive,
            // and aren't made sluggish by unnecessarily filtering out noise. A hyperbola (f(x) = 1/x) curve is used.
            // First x has an offset of 1 applied, so x = 0 now results in a value of 1 from the hyperbola function.
            // High values of x tend toward 0, but we want an output that begins at 0 and tends toward 1, so 1-y flips this up the right way.
            // Finally the result is multiplied by 2 and capped at a maximum of one, which means that at a certain point all larger movements are maximally snappy

            // then multiply the input by SNAP_MULTIPLER so input values fit the snap curve better.
            auto snap = snapCurve(diff * mSnapMultiplier);

            // when sleep is enabled, the emphasis is stopping on a responsiveValue quickly, and it's less about easing into position.
            // If sleep is enabled, add a small amount to snap so it'll tend to snap into a more accurate position before sleeping starts.
            if(mSleepEnabled)
            {
              snap *= 0.5 + 0.5;
            }

            // calculate the exponential moving average based on the snap
            mSmoothValue += (newValue - mSmoothValue) * snap;

            // ensure output is in bounds
            if(mSmoothValue < mLowerBound
               || (mEdgeSnapEnabled
                   && std::abs(mSmoothValue - mLowerBound) < mActivityThreshold))
            {
              mSmoothValue = mLowerBound;
            }
            if (mSmoothValue > mUpperBound
                || (mEdgeSnapEnabled
                    && std::abs(mSmoothValue - mUpperBound) < mActivityThreshold))
            {
              mSmoothValue = mUpperBound;
            }
        }

        return mSmoothValue;
    }

    double snapCurve(double x)
    {
        double y = 1.0 / (x + 1.0);
        y = (1.0 - y) * 2.0;

        if(y > 1.0)
        {
          y = 1.0;
        }

        return y;
    }

    bool mEnabled;
    bool mFirstValue;
    bool mSleepEnabled;
    bool mEdgeSnapEnabled;
    bool mSleeping;

    T mSmoothValue;
    double mErrorEMA;

    T mLowerBound;
    T mUpperBound;


    T mRawValue;
    T mFilteredValue;
    T mPrevResponsiveValue;
    bool mFilteredValueHasChanged;

    double mSnapMultiplier;
    double mActivityThreshold;
};

#endif
