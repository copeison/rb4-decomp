#pragma once

#include <math.h>

// The easing curves (math/Easing.o). The map's object holds only the bounce
// curve (EaseBounceOut and EaseBounceOutInv, which take three floats); in
// this build the curves are inline and the key tracks' easing dispatch
// (InterpolationEase, entity/waveform/Key.h) inlines them, except the
// elastic in/out pair, emitted beside it. Every curve maps 0 to 0 and 1 to
// 1. Names not in the reference map unless noted.

// Reads the math easing_parameters block of the system configuration:
// ease_back_overshoot, ease_elastic_amplitude and the other curve settings.
// Name not in the reference map.
void InitEasingParameters();  // 0x2122F0

// The curve settings InitEasingParameters reads. Names not in the reference
// map.
extern float gEaseBackOvershoot;
extern float gEaseElasticAmplitude;
extern float gEaseElasticPeriod;
extern float gEaseStairStepPower;
extern float gEasePolynomialPower;

// The in/out forms of an ease-in curve and its ease-out counterpart: the
// first half eases in (or out) at double speed and the second half
// continues with the other curve.
template <class In, class Out>
inline float EaseInOut(float t, In in, Out out) {
    if (t < 0.5F) {
        return in(t + t) * 0.5F;
    }
    return (out(t + t - 1.0F) + 1.0F) * 0.5F;
}

template <class In, class Out>
inline float EaseOutIn(float t, In in, Out out) {
    if (t < 0.5F) {
        return out(t + t) * 0.5F;
    }
    return (in(t + t - 1.0F) + 1.0F) * 0.5F;
}

// The curves of the whole powers from 2 to 5, and their mirrors.
inline float EaseQuadraticIn(float t) {
    return t * t;
}

inline float EaseQuadraticOut(float t) {
    const float u = 1.0F - t;
    return 1.0F - u * u;
}

inline float EaseCubicIn(float t) {
    return t * t * t;
}

inline float EaseCubicOut(float t) {
    const float u = 1.0F - t;
    return 1.0F - u * u * u;
}

inline float EaseQuarticIn(float t) {
    const float square = t * t;
    return square * square;
}

inline float EaseQuarticOut(float t) {
    const float u = 1.0F - t;
    const float square = u * u;
    return 1.0F - square * square;
}

inline float EaseQuinticIn(float t) {
    const float square = t * t;
    return square * square * t;
}

inline float EaseQuinticOut(float t) {
    const float u = 1.0F - t;
    const float square = u * u;
    return 1.0F - square * square * u;
}

// The polynomial curves: t to the configured power, and its mirror.
inline float EasePowerIn(float t, float power) {
    return powf(t, power);
}

inline float EasePowerOut(float t, float power) {
    return 1.0F - powf(1.0F - t, power);
}

// The back curve overshoots below 0 before rising, by the overshoot.
inline float EaseBackIn(float t, float overshoot) {
    return t * t * ((overshoot + 1.0F) * t - overshoot);
}

inline float EaseBackOut(float t, float overshoot) {
    return 1.0F - EaseBackIn(1.0F - t, overshoot);
}

// The bounce curve: four parabolic arcs of decreasing height. The map has
// EaseBounceOut(float, float, float) in math/Easing.o.
inline float EaseBounceOut(float t) {
    if (t < 1.0F / 2.75F) {
        return 7.5625F * t * t;
    }
    if (t < 2.0F / 2.75F) {
        t -= 1.5F / 2.75F;
        return 7.5625F * t * t + 0.75F;
    }
    if (t < 2.5 / 2.75) {
        t -= 2.25F / 2.75F;
        return 7.5625F * t * t + 0.9375F;
    }
    t -= 2.625F / 2.75F;
    return 7.5625F * t * t + 0.984375F;
}

inline float EaseBounceIn(float t) {
    return 1.0F - EaseBounceOut(1.0F - t);
}

// The circular curves: quarter circles.
inline float EaseCircularIn(float t) {
    return 1.0F - sqrtf(1.0F - t * t);
}

inline float EaseCircularOut(float t) {
    t -= 1.0F;
    return sqrtf(1.0F - t * t);
}

// The elastic curve: a sine wave of the period whose amplitude grows
// exponentially to the end. Outside (0, 1) it returns t. An amplitude
// below 1 is taken as 1; a period that is not positive is 0.45.
inline float EaseElasticIn(float t, float amplitude, float period) {
    if (t <= 0.0F || t >= 1.0F) {
        return t;
    }
    period = period > 0.0F ? period : 0.45F;
    float shift;
    if (amplitude >= 1.0F) {
        shift = period * (1.0F / (2.0F * 3.14159265F)) * asinf(1.0F / amplitude);
    } else {
        shift = period * 0.25F;
        amplitude = 1.0F;
    }
    t -= 1.0F;
    return -(amplitude * exp2f(t * 10.0F) * sinf((t - shift) * (2.0F * 3.14159265F) / period));
}

inline float EaseElasticOut(float t, float amplitude, float period) {
    return 1.0F - EaseElasticIn(1.0F - t, amplitude, period);
}

// The elastic in/out forms, which the binary does not inline.
inline float EaseElasticInOut(float t, float amplitude, float period) {
    if (t < 0.5F) {
        return EaseElasticIn(t + t, amplitude, period) * 0.5F;
    }
    return (EaseElasticOut(t + t - 1.0F, amplitude, period) + 1.0F) * 0.5F;
}  // 0x1CF650

inline float EaseElasticOutIn(float t, float amplitude, float period) {
    if (t < 0.5F) {
        return EaseElasticOut(t + t, amplitude, period) * 0.5F;
    }
    return (EaseElasticIn(t + t - 1.0F, amplitude, period) + 1.0F) * 0.5F;
}  // 0x1CF8C0

// The exponential curves, offset so they reach 0 and 1 at the ends.
inline float EaseExponentialIn(float t) {
    return t == 0.0F ? 0.0F : exp2f((t - 1.0F) * 10.0F) - 0.001F;
}

inline float EaseExponentialOut(float t) {
    return t == 1.0F ? 1.0F : (1.0F - exp2f(t * -10.0F)) * 1.001F;
}

// The sine curves: a quarter period.
inline float EaseSineIn(float t) {
    return 1.0F - cosf(t * (3.14159265F / 2.0F));
}

inline float EaseSineOut(float t) {
    return sinf(t * (3.14159265F / 2.0F));
}

// Steps that ease between whole values: the scaled time's fraction follows
// the polynomial in/out curve of the stair-step power.
inline float EaseStairStep(float t, float steps, float power) {
    const float scaled = steps * t;
    const float step = floorf(scaled);
    const float fraction = scaled - step;
    float eased;
    if (fraction < 0.5F) {
        eased = powf(fraction + fraction, power);
    } else {
        eased = 2.0F - powf(2.0F - (fraction + fraction), power);
    }
    return (eased * 0.5F + step) / steps;
}
