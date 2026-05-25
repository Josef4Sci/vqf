// SPDX-FileCopyrightText: 2026
//
// SPDX-License-Identifier: MIT

#ifndef STATIC_DETECTOR_HPP
#define STATIC_DETECTOR_HPP

#include "vqf.hpp"

#include <deque>
#include <cmath>
#include <iostream>

class StaticDetector
{
public:
    StaticDetector(vqf_real_t accThreshold=vqf_real_t(0.1),
                   vqf_real_t gyrThreshold=vqf_real_t(0.015),
                   vqf_real_t magThreshold=vqf_real_t(3.0),
                   size_t windowSize=3,
                   size_t blockForwardSteps=500)
        : accThreshold(accThreshold)
        , gyrThreshold(gyrThreshold)
        , magThreshold(magThreshold)
        , windowSize(windowSize)
        , blockForwardSteps(blockForwardSteps)
        , sampleIndex(-1)
        , lastMotionIndex(-static_cast<long>(blockForwardSteps))
    {
        // std::cout << "StaticDetector initialized with accThreshold=" << accThreshold
        //           << ", gyrThreshold=" << gyrThreshold
        //           << ", magThreshold=" << magThreshold
        //           << ", windowSize=" << windowSize
        //           << ", blockForwardSteps=" << blockForwardSteps;
    }

    void reset()
    {
        accBuffer.clear();
        gyrBuffer.clear();
        magBuffer.clear();
        sampleIndex = -1;
        lastMotionIndex = -static_cast<long>(blockForwardSteps);
    }

    bool update(const vqf_real_t acc[3], const vqf_real_t gyr[3], const vqf_real_t mag[3])
    {
        sampleIndex++;
        accBuffer.push_back({acc[0], acc[1], acc[2]});
        gyrBuffer.push_back({gyr[0], gyr[1], gyr[2]});
        magBuffer.push_back({mag[0], mag[1], mag[2]});

        if (accBuffer.size() > windowSize) {
            accBuffer.pop_front();
            gyrBuffer.pop_front();
            magBuffer.pop_front();
        }

        const bool staticDetected = isStaticAcc() && isStaticGyr() && isStaticMag();
        if (!staticDetected) {
            lastMotionIndex = sampleIndex;
        }
        return isStatic();
    }

    bool isStatic() const
    {
        if (sampleIndex - lastMotionIndex < static_cast<long>(blockForwardSteps)) {
            return false;
        }
        return isStaticAcc() && isStaticGyr() && isStaticMag();
    }

private:
    struct Vec3 {
        vqf_real_t x;
        vqf_real_t y;
        vqf_real_t z;
    };

    static vqf_real_t vecNorm(const Vec3& v)
    {
        return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    }

    static Vec3 vecDiff(const Vec3& a, const Vec3& b)
    {
        Vec3 out;
        out.x = a.x - b.x;
        out.y = a.y - b.y;
        out.z = a.z - b.z;
        return out;
    }

    vqf_real_t gyrVariance() const
    {
        if (gyrBuffer.empty()) {
            return vqf_real_t(0.0);
        }

        vqf_real_t s = vqf_real_t(0.0);
        for (size_t i = 0; i < gyrBuffer.size(); i++) {
            s += vecNorm(gyrBuffer[i]);
        }
        return s/static_cast<vqf_real_t>(gyrBuffer.size());
    }

    vqf_real_t accVariance() const
    {
        if (accBuffer.size() < 2) {
            return vqf_real_t(0.0);
        }

        vqf_real_t s = vqf_real_t(0.0);
        for (size_t i = 1; i < accBuffer.size(); i++) {
            s += vecNorm(vecDiff(accBuffer[i], accBuffer[i-1]));
        }
        return s/static_cast<vqf_real_t>(accBuffer.size()-1);
    }

    vqf_real_t magVariance() const
    {
        if (magBuffer.size() < 2) {
            return vqf_real_t(0.0);
        }

        vqf_real_t s = vqf_real_t(0.0);
        for (size_t i = 1; i < magBuffer.size(); i++) {
            s += vecNorm(vecDiff(magBuffer[i], magBuffer[i-1]));
        }
        return s/static_cast<vqf_real_t>(magBuffer.size()-1);
    }

    bool isStaticAcc() const
    {
        if (accBuffer.size() < windowSize) {
            return false;
        }
        return accVariance() < accThreshold;
    }

    bool isStaticGyr() const
    {
        if (gyrBuffer.size() < windowSize) {
            return false;
        }
        return gyrVariance() < gyrThreshold;
    }

    bool isStaticMag() const
    {
        if (magBuffer.size() < windowSize) {
            return false;
        }
        return magVariance() < magThreshold;
    }

    vqf_real_t accThreshold;
    vqf_real_t gyrThreshold;
    vqf_real_t magThreshold;
    size_t windowSize;
    size_t blockForwardSteps;

    std::deque<Vec3> accBuffer;
    std::deque<Vec3> gyrBuffer;
    std::deque<Vec3> magBuffer;

    long sampleIndex;
    long lastMotionIndex;
};

#endif // STATIC_DETECTOR_HPP