//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <nlohmann/json.hpp>

namespace Benchmark
{
    struct Statistics
    {
        double Mean = 0.0;
        double Median = 0.0;
        double StdDev = 0.0;
        double Min = 0.0;
        double Max = 0.0;
        double P1 = 0.0;
        double P5 = 0.0;
        double P95 = 0.0;
        double P99 = 0.0;
        uint32_t SampleCount = 0;
        double CoefficientOfVariation = 0.0;

        static Statistics Compute(const std::vector<float>& samples)
        {
            Statistics stats;
            if (samples.empty())
                return stats;

            stats.SampleCount = static_cast<uint32_t>(samples.size());

            std::vector<float> sorted = samples;
            std::sort(sorted.begin(), sorted.end());

            stats.Min = sorted.front();
            stats.Max = sorted.back();

            double sum = 0.0;
            for (float v : samples)
                sum += v;
            stats.Mean = sum / samples.size();

            size_t mid = samples.size() / 2;
            if (samples.size() % 2 == 0)
                stats.Median = (sorted[mid - 1] + sorted[mid]) / 2.0;
            else
                stats.Median = sorted[mid];

            double variance = 0.0;
            for (float v : samples)
            {
                double diff = v - stats.Mean;
                variance += diff * diff;
            }
            variance /= samples.size();
            stats.StdDev = std::sqrt(variance);

            if (stats.Mean > 0.0)
                stats.CoefficientOfVariation = stats.StdDev / stats.Mean;

            auto percentile = [&sorted](double p) -> double {
                double index = (p / 100.0) * (sorted.size() - 1);
                size_t lower = static_cast<size_t>(index);
                size_t upper = lower + 1;
                if (upper >= sorted.size())
                    return sorted.back();
                double frac = index - lower;
                return sorted[lower] * (1.0 - frac) + sorted[upper] * frac;
            };

            stats.P1 = percentile(1.0);
            stats.P5 = percentile(5.0);
            stats.P95 = percentile(95.0);
            stats.P99 = percentile(99.0);

            return stats;
        }

        nlohmann::json ToJson() const
        {
            return {
                    {"mean", Mean},
                    {"median", Median},
                    {"std_dev", StdDev},
                    {"min", Min},
                    {"max", Max},
                    {"p1", P1},
                    {"p5", P5},
                    {"p95", P95},
                    {"p99", P99},
                    {"sample_count", SampleCount},
                    {"coefficient_of_variation", CoefficientOfVariation}
            };
        }
    };
}