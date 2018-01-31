#ifndef SIMPLE_NOISE_FILTER_H
#define SIMPLE_NOISE_FILTER_H

#include <math.h>


template<class T>
class SimpleNoiseFilter
{
  public:
    SimpleNoiseFilter(T threshold,
                      int suppressCount):
        mEnabled(true),
        mFirstValue(true),
        mActivityThreshold(threshold),
        mSuppressionCount(suppressCount),
        mCurrentSuppressionCount(0)
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

    int suppressionCount() const
    {
        return mSuppressionCount;
    }

    void setSuppressionCount(int count)
    {
        mSuppressionCount = count;
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

        //If the change is greater than activity threshold,
        //Try suppressing it
        if (std::abs(newValue - mSmoothValue) > mActivityThreshold)
        {
            mCurrentSuppressionCount++;

            //If unable to suppress, The new value is the average of the old and new.
            if(mCurrentSuppressionCount > mSuppressionCount)
            {
                mSmoothValue = (mSmoothValue + newValue)/2.0;
                mCurrentSuppressionCount = 0;
            }
        }
        else
        {
            //Else the new value is the average of old and new value
            mSmoothValue = (mSmoothValue + newValue)/2.0;
            mCurrentSuppressionCount = 0;
        }

        return mSmoothValue;
    }


    bool mEnabled;
    bool mFirstValue;

    T mPrevResponsiveValue;
    T mSmoothValue;
    T mRawValue;
    T mFilteredValue;

    bool mFilteredValueHasChanged;

    T mActivityThreshold;
    int mSuppressionCount;
    int mCurrentSuppressionCount;
};

#endif
