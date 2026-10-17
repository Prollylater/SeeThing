#ifndef CORE_STROKE_H
#define CORE_STROKE_H

#include "Geometry.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace core
{
    // Places dabs along a pointer path at an even spacing. The distance walked since the last dab carries over
    // from one segment to the next, so frame boundaries don't show in the stroke.
    class DabSpacer
    {
    public:
        // The first dab sits on p
        void begin(Vec2f p, std::vector<Vec2f> &out)
        {
            m_last = p;
            m_travelled = 0.0f;
            out.push_back(p);
        }

        void moveTo(Vec2f p, float spacing, std::vector<Vec2f> &out)
        {
            spacing = std::max(spacing, 0.1f);
            const Vec2f d = p - m_last;
            const float len = std::hypot(d.x, d.y);
            if (len <= 0.0f)
                return;
            // Distance along this segment to the next dab
            float t = spacing - m_travelled;
            for (; t <= len; t += spacing)
                out.push_back(m_last + d * (t / len));
            m_travelled = len - (t - spacing);
            m_last = p;
        }

    private:
        Vec2f m_last;
        float m_travelled = 0.0f;
    };

    // Exponential moving average: 0 follows the pointer, near 1 trails behind it and irons out the shake
    inline Vec2f smoothTowards(Vec2f current, Vec2f target, float smoothing)
    {
        return current + (target - current) * (1.0f - std::clamp(smoothing, 0.0f, 0.95f));
    }
}

#endif
