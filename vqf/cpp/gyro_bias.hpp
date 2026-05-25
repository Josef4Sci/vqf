// SPDX-FileCopyrightText: 2026
//
// SPDX-License-Identifier: MIT

#ifndef GYRO_BIAS_HPP
#define GYRO_BIAS_HPP

#include "vqf.hpp"

#include <deque>

class GyroBias
{
public:
    explicit GyroBias(size_t bufferSize = 500, bool fast = false, vqf_real_t iirAlpha = vqf_real_t(0.035))
        : fast(fast), bufferSize(bufferSize), iirAlpha(iirAlpha), iirInitialized(false)
    {
        iirBias[0] = iirBias[1] = iirBias[2] = vqf_real_t(0.0);
    }

    void reset()
    {
        xBuffer.clear();
        yBuffer.clear();
        zBuffer.clear();
        iirInitialized = false;
        iirBias[0] = iirBias[1] = iirBias[2] = vqf_real_t(0.0);
    }

    void update(const vqf_real_t gyroscope[3])
    {
        if (fast)
        {
            const vqf_real_t oneMinusAlpha = vqf_real_t(1.0) - iirAlpha;
            iirBias[0] = oneMinusAlpha * iirBias[0] + iirAlpha * gyroscope[0];
            iirBias[1] = oneMinusAlpha * iirBias[1] + iirAlpha * gyroscope[1];
            iirBias[2] = oneMinusAlpha * iirBias[2] + iirAlpha * gyroscope[2];

            return;
        }

        push(xBuffer, gyroscope[0]);
        push(yBuffer, gyroscope[1]);
        push(zBuffer, gyroscope[2]);
    }

    void getBias(vqf_real_t out[3]) const
    {
        if (fast)
        {
            out[0] = iirInitialized ? iirBias[0] : vqf_real_t(0.0);
            out[1] = iirInitialized ? iirBias[1] : vqf_real_t(0.0);
            out[2] = iirInitialized ? iirBias[2] : vqf_real_t(0.0);
            return;
        }

        out[0] = mean(xBuffer);
        out[1] = mean(yBuffer);
        out[2] = mean(zBuffer);
    }

private:
    static vqf_real_t mean(const std::deque<vqf_real_t> &buffer)
    {
        if (buffer.empty())
        {
            return vqf_real_t(0.0);
        }

        vqf_real_t sum = vqf_real_t(0.0);
        for (size_t i = 0; i < buffer.size(); i++)
        {
            sum += buffer[i];
        }
        return sum / static_cast<vqf_real_t>(buffer.size());
    }

    void push(std::deque<vqf_real_t> &buffer, vqf_real_t value)
    {
        buffer.push_back(value);
        if (buffer.size() > bufferSize)
        {
            buffer.pop_front();
        }
    }

    bool fast;
    size_t bufferSize;
    vqf_real_t iirAlpha;
    bool iirInitialized;
    vqf_real_t iirBias[3];
    std::deque<vqf_real_t> xBuffer;
    std::deque<vqf_real_t> yBuffer;
    std::deque<vqf_real_t> zBuffer;
};

#endif // GYRO_BIAS_HPP