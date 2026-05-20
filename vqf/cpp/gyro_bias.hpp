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
    explicit GyroBias(size_t bufferSize=500)
        : bufferSize(bufferSize)
    {
    }

    void reset()
    {
        xBuffer.clear();
        yBuffer.clear();
        zBuffer.clear();
    }

    void update(const vqf_real_t gyroscope[3])
    {
        push(xBuffer, gyroscope[0]);
        push(yBuffer, gyroscope[1]);
        push(zBuffer, gyroscope[2]);
    }

    void getBias(vqf_real_t out[3]) const
    {
        out[0] = mean(xBuffer);
        out[1] = mean(yBuffer);
        out[2] = mean(zBuffer);
    }

private:
    static vqf_real_t mean(const std::deque<vqf_real_t>& buffer)
    {
        if (buffer.empty()) {
            return vqf_real_t(0.0);
        }

        vqf_real_t sum = vqf_real_t(0.0);
        for (size_t i = 0; i < buffer.size(); i++) {
            sum += buffer[i];
        }
        return sum/static_cast<vqf_real_t>(buffer.size());
    }

    void push(std::deque<vqf_real_t>& buffer, vqf_real_t value)
    {
        buffer.push_back(value);
        if (buffer.size() > bufferSize) {
            buffer.pop_front();
        }
    }

    size_t bufferSize;
    std::deque<vqf_real_t> xBuffer;
    std::deque<vqf_real_t> yBuffer;
    std::deque<vqf_real_t> zBuffer;
};

#endif // GYRO_BIAS_HPP