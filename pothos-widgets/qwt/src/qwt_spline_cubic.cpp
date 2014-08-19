/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_cubic.h"
#include <qdebug.h>

#define SLOPES_INCREMENTAL 0
#define KAHAN 0

namespace QwtSplineCubicP
{
    class KahanSum 
    {
    public:
        inline KahanSum( double value = 0.0 ):
            d_sum( value ),
            d_carry( 0.0 )
        {
        }

        inline void reset()
        {
            d_sum = d_carry = 0.0;
        }

        inline double value() const 
        {
            return d_sum;
        }

        inline void add( double value )
        {
            const double y = value - d_carry;
            const double t = d_sum + y;

            d_carry = ( t - d_sum ) - y;
            d_sum = t;
        }

        static inline double sum3( double d1, double d2, double d3 )
        {
            KahanSum sum( d1 );
            sum.add( d2 );
            sum.add( d3 );

            return sum.value();
        }

        static inline double sum4( double d1, double d2, double d3, double d4 )
        {
            KahanSum sum( d1 );
            sum.add( d2 );
            sum.add( d3 );
            sum.add( d4 );

            return sum.value();
        }   


    private:
        double d_sum; 
        double d_carry; // The carry from the previous operation
    };

    class CurvatureStore
    {
    public:
        inline void setup( int size )
        {
            d_curvatures.resize( size );
            d_cv = d_curvatures.data();
        }

        inline void storeFirst( double, 
            const QPointF &, const QPointF &, double b1, double )
        {
            d_cv[0] = 2.0 * b1;
        }

        inline void storeNext( int index, double,
            const QPointF &, const QPointF &, double, double b2 )
        {
            d_cv[index] = 2.0 * b2;
        }

        inline void storeLast( double,
            const QPointF &, const QPointF &, double, double b2 )
        {
            d_cv[d_curvatures.size() - 1] = 2.0 * b2;
        }

        inline void storePrevious( int index, double,
            const QPointF &, const QPointF &, double b1, double )
        {
            d_cv[index] = 2.0 * b1;
        }

        inline void closeR()
        {
            d_cv[0] = d_cv[d_curvatures.size() - 1];
        }

        QVector<double> curvatures() const { return d_curvatures; }

    private:
        QVector<double> d_curvatures;
        double *d_cv;
    };

    class SlopeStore
    {
    public:
        inline void setup( int size )
        {
            d_slopes.resize( size );
            d_m = d_slopes.data();
        }
        
        inline void storeFirst( double h, 
            const QPointF &p1, const QPointF &p2, double b1, double b2 )
        {  
            const double s = ( p2.y() - p1.y() ) / h;
            d_m[0] = s - h * ( 2.0 * b1 + b2 ) / 3.0;
#if KAHAN
            d_sum.add( d_m[0] );
#endif
        }
        
        inline void storeNext( int index, double h,
            const QPointF &p1, const QPointF &p2, double b1, double b2 )
        {
#if SLOPES_INCREMENTAL
            Q_UNUSED( p1 )
            Q_UNUSED( p2 )
#if KAHAN
            d_sum.add( ( b1 + b2 ) * h );
            d_m[index] = d_sum.value();
#else
            d_m[index] = d_m[index-1] + ( b1 + b2 ) * h;
#endif
#else
            const double s = ( p2.y() - p1.y() ) / h;
            d_m[index] = s + h * ( b1 + 2.0 * b2 ) / 3.0;
#endif
        }

        inline void storeLast( double h,
            const QPointF &p1, const QPointF &p2, double b1, double b2 )
        {   
            const double s = ( p2.y() - p1.y() ) / h;
            d_m[d_slopes.size() - 1] = s + h * ( b1 + 2.0 * b2 ) / 3.0;
#if KAHAN
            d_sum.add( d_m[d_slopes.size() - 1] );
#endif
        }

        inline void storePrevious( int index, double h,
            const QPointF &p1, const QPointF &p2, double b1, double b2 )
        {
#if SLOPES_INCREMENTAL
            Q_UNUSED( p1 )
            Q_UNUSED( p2 )
#if KAHAN
            d_sum.add( -( b1 + b2 ) * h );
            d_m[index] = d_sum.value();
#else
            d_m[index] = d_m[index+1] - ( b1 + b2 ) * h;
#endif

#else
            const double s = ( p2.y() - p1.y() ) / h;
            d_m[index] = s - h * ( 2.0 * b1 + b2 ) / 3.0;
#endif
        }

        inline void closeR()
        {
            d_m[0] = d_m[d_slopes.size() - 1];
        }

        QVector<double> slopes() const { return d_slopes; }
    
    private:
        QVector<double> d_slopes;
        double *d_m;
#if SLOPES_INCREMENTAL
        KahanSum d_sum;
#endif
    };
};

namespace QwtSplineCubicP
{
    class Equation2
    {
    public:
        inline Equation2()
        {
        }

        inline Equation2( double p0, double q0, double r0 ):
            p( p0 ),
            q( q0 ),
            r( r0 )
        {
        }

        inline void setup( double p0 , double q0, double r0 )
        {
            p = p0;
            q = q0;
            r = r0;
        }
            
        inline Equation2 normalized() const
        {
            Equation2 c;
            c.p = 1.0;
            c.q = q / p;
            c.r = r / p;

            return c;
        }

        inline double resolved1( double x2 ) const
        {
            return ( r - q * x2 ) / p;
        }

        inline double resolved2( double x1 ) const
        {
            return ( r - p * x1 ) / q;
        }

        inline double resolved1( const Equation2 &eq ) const
        {
            // find x1
            double k = q / eq.q;
            return ( r - k * eq.r ) / ( p - k * eq.p );
        }

        inline double resolved2( const Equation2 &eq ) const
        {
            // find x2
            const double k = p / eq.p;
            return ( r - k * eq.r ) / ( q - k * eq.q );
        }

        // p * x1 + q * x2 = r
        double p, q, r;
    };

    class Equation3
    {
    public:
        inline Equation3()
        {
        }

        inline Equation3( const QPointF &p1, const QPointF &p2, const QPointF &p3 )
        {
            const double h1 = p2.x() - p1.x();
            const double s1  = ( p2.y() - p1.y() ) / h1;

            const double h2 = p3.x() - p2.x();
            const double s2  = ( p3.y() - p2.y() ) / h2;

            p = h1;
            q = 2 * ( h1 + h2 );
            u = h2;
            r = 3 * ( s2 - s1 );
        }

        inline Equation3( double cp, double cq, double du, double dr ):
            p( cp ),
            q( cq ),
            u( du ),
            r( dr )
        {
        }
    
        inline bool operator==( const Equation3 &c ) const
        {
            return ( p == c.p ) && ( q == c.q ) && 
                ( u == c.u ) && ( r == c.r );
        }

        inline void setup( double cp, double cq, double du, double dr )
        {
            p = cp;
            q = cq;
            u = du;
            r = dr;
        }

        inline Equation3 normalized() const
        {
            Equation3 c;
            c.p = 1.0;
            c.q = q / p;
            c.u = u / p;
            c.r = r / p;

            return c;
        }

        inline Equation2 substituted1( const Equation3 &eq ) const
        {
            // eliminate x1
            const double k = p / eq.p;
            return Equation2( q - k * eq.q, u - k * eq.u, r - k * eq.r );
        }

        inline Equation2 substituted2( const Equation3 &eq ) const
        {
            // eliminate x2

            const double k = q / eq.q;
            return Equation2( p - k * eq.p, u - k * eq.u, r - k * eq.r );
        }

        inline Equation2 substituted3( const Equation3 &eq ) const
        {
            // eliminate x3

            const double k = u / eq.u;
            return Equation2( p - k * eq.p, q - k * eq.q, r - k * eq.r );
        }

        inline Equation2 substituted1( const Equation2 &eq ) const
        {
            // eliminate x1
            const double k = p / eq.p;
            return Equation2( q - k * eq.q, u, r - k * eq.r );
        }

        inline Equation2 substituted3( const Equation2 &eq ) const
        {
            // eliminate x3

            const double k = u / eq.q;
            return Equation2( p, q - k * eq.p, r - k * eq.r );
        }


        inline double resolved1( double x2, double x3 ) const
        {
            return ( r - q * x2 - u * x3 ) / p;
        }

        inline double resolved2( double x1, double x3 ) const
        {
            return ( r - u * x3 - p * x1 ) / q;
        }

        inline double resolved3( double x1, double x2 ) const
        {
            return ( r - p * x1 - q * x2 ) / u;
        }

        // p * x1 + q * x2 + u * x3 = r
        double p, q, u, r;
    };
};
         
QDebug operator<<( QDebug debug, const QwtSplineCubicP::Equation2 &eq )
{
    debug.nospace() << "EQ2(" << eq.p << ", " << eq.q << ", " << eq.r << ")";
    return debug.space();
}

QDebug operator<<( QDebug debug, const QwtSplineCubicP::Equation3 &eq )
{
    debug.nospace() << "EQ3(" << eq.p << ", " 
        << eq.q << ", " << eq.u << ", " << eq.r << ")";
    return debug.space();
}

namespace QwtSplineCubicP
{
    template <class T>
    class EquationSystem
    {
    public:
        void setStartCondition( double p, double q, double u, double r )
        {
            d_conditionsEQ[0].setup( p, q, u, r );
        }

        void setEndCondition( double p, double q, double u, double r )
        {
            d_conditionsEQ[1].setup( p, q, u, r );
        }

        const T &store() const 
        { 
            return d_store;
        }

        void resolve( const QPolygonF &p ) 
        {
            const int n = p.size();
            if ( n < 3 )
                return;

            if ( d_conditionsEQ[0].p == 0.0 ||
                ( d_conditionsEQ[0].q == 0.0 && d_conditionsEQ[0].u != 0.0 ) )
            {
                return;
            }

            if ( d_conditionsEQ[1].u == 0.0 ||
                ( d_conditionsEQ[1].q == 0.0 && d_conditionsEQ[1].p != 0.0 ) )
            {
                return;
            }

            const double h0 = p[1].x() - p[0].x();
            const double h1 = p[2].x() - p[1].x();
            const double hn = p[n-1].x() - p[n-2].x();

            d_store.setup( n );
            

            if ( n == 3 )
            {
                // For certain conditions the first/last point does not 
                // necessarily meet the spline equation and we would
                // have many solutions. In this case we resolve using
                // the spline equation - as for all other conditions.

                const Equation3 eqSpline0( p[0], p[1], p[2] ); // ???
                const Equation2 eq0 = d_conditionsEQ[0].substituted1( eqSpline0 );

                // The equation system can be solved without substitution
                // from the start/end conditions and eqSpline0 ( = eqSplineN ).

                double b1;

                if ( d_conditionsEQ[0].normalized() == d_conditionsEQ[1].normalized() )
                {
                    // When we have 3 points only and start/end conditions
                    // for 3 points mean the same condition the system
                    // is under-determined and has many solutions.
                    // We chose b1 = 0.0

                    b1 = 0.0;
                }
                else 
                {
                    const Equation2 eq = d_conditionsEQ[1].substituted1( eqSpline0 );
                    b1 = eq0.resolved1( eq );
                }

                const double b2 = eq0.resolved2( b1 );
                const double b0 = eqSpline0.resolved1( b1, b2 );

                d_store.storeFirst( h0, p[0], p[1], b0, b1 );
                d_store.storeNext( 1, h0, p[0], p[1], b0, b1 );
                d_store.storeNext( 2, h1, p[1], p[2], b1, b2 );

                return; 
            }

            const Equation3 eqSplineN( p[n-3], p[n-2], p[n-1] );
            const Equation2 eqN = d_conditionsEQ[1].substituted3( eqSplineN );

            Equation2 eq = eqN;
            if ( n > 4 )
            {
                const Equation3 eqSplineR( p[n-4], p[n-3], p[n-2] );
                eq = eqSplineR.substituted3( eq );
                eq = substituteSpline( p, eq );
            }

            const Equation3 eqSpline0( p[0], p[1], p[2] ); 

            double b0, b1;
            if ( d_conditionsEQ[0].u == 0.0 )
            {
                eq = eqSpline0.substituted3( eq );

                const Equation3 &eq0 = d_conditionsEQ[0];
                b0 = Equation2( eq0.p, eq0.q, eq0.r ).resolved1( eq );
                b1 = eq.resolved2( b0 );
            }
            else
            {
                const Equation2 eqX = d_conditionsEQ[0].substituted3( eq );
                const Equation2 eqY = eqSpline0.substituted3( eq );

                b0 = eqY.resolved1( eqX );
                b1 = eqY.resolved2( b0 );
            }

            d_store.storeFirst( h0, p[0], p[1], b0, b1 );
            d_store.storeNext( 1, h0, p[0], p[1], b0, b1 );

            const double bn2 = resolveSpline( p, b1 );

            const double bn1 = eqN.resolved2( bn2 );
            const double bn0 = d_conditionsEQ[1].resolved3( bn2, bn1 );

            const double hx = p[n-2].x() - p[n-3].x();
            d_store.storeNext( n - 2, hx, p[n-3], p[n-2], bn2, bn1 );
            d_store.storeNext( n - 1, hn, p[n-2], p[n-1], bn1, bn0 );
        }

    private:
        Equation2 substituteSpline( const QPolygonF &points, const Equation2 &eq )
        {
            const int n = points.size();

            d_eq.resize( n - 2 );
            d_eq[n-3] = eq;

            // eq[i].resolved2( b[i-1] ) => b[i]

            double slope2 = ( points[n-3].y() - points[n-4].y() ) / eq.p;

            for ( int i = n - 4; i > 1; i-- )
            {
                const Equation2 &eq2 = d_eq[i+1];
                Equation2 &eq1 = d_eq[i];

                eq1.p = points[i].x() - points[i-1].x();
                const double slope1 = ( points[i].y() - points[i-1].y() ) / eq1.p;

                const double v = eq2.p / eq2.q;

                eq1.q = 2.0 * ( eq1.p + eq2.p ) - v * eq2.p;
                eq1.r = 3.0 * ( slope2 - slope1 ) - v * eq2.r;

                slope2 = slope1;
            }

            return d_eq[2];
        }

        double resolveSpline( const QPolygonF &points, double b1 )
        {
            const int n = points.size();
            const QPointF *p = points.constData();

            for ( int i = 2; i < n - 2; i++ )
            {
                // eq[i].resolved2( b[i-1] ) => b[i]
                const double b2 = d_eq[i].resolved2( b1 );
                d_store.storeNext( i, d_eq[i].p, p[i-1], p[i], b1, b2 );

                b1 = b2;
            }

            return b1;
        }

    private:
        Equation3 d_conditionsEQ[2];
        QVector<Equation2> d_eq;
        T d_store;
    };

    template <class T>
    class EquationSystem2
    {
    public:
        const T &store() const
        {
            return d_store;
        }

        void resolve( const QPolygonF &p )
        {
            const int n = p.size();

            if ( p[n-1].y() != p[0].y() )
            {
                // TODO ???
            }

            const double h0 = p[1].x() - p[0].x();
            const double s0 = ( p[1].y() - p[0].y() ) / h0;

            if ( n == 3 )
            {
                const double h1 = p[2].x() - p[1].x();
                const double s1 = ( p[2].y() - p[1].y() ) / h1;

                const double b = 3.0 * ( s0 - s1 ) / ( h0 + h1 );

                d_store.setup( 3 );
                d_store.storeLast( h1, p[1], p[2], -b, b );
                d_store.storePrevious( 1, h1, p[1], p[2], -b, b );
                d_store.closeR();

                return;
            }

            const double hn = p[n-1].x() - p[n-2].x();

            Equation2 eqn, eqX;
            substitute( p, eqn, eqX );

            const double b0 = eqn.resolved2( eqX );
            const double bn = eqn.resolved1( b0 );

            d_store.setup( n );
            d_store.storeLast( hn, p[n-2], p[n-1], bn, b0 );
            d_store.storePrevious( n - 2, hn, p[n-2], p[n-1], bn, b0 );

            resolveSpline( p, b0, bn );

            d_store.closeR();
        }

    private:

        void substitute( const QPolygonF &points, Equation2 &eqn, Equation2 &eqX )
        {
            const int n = points.size();

            const double hn = points[n-1].x() - points[n-2].x();

            const Equation3 eqSpline0( points[0], points[1], points[2] );
            const Equation3 eqSplineN( 
                QPointF( points[0].x() - hn, points[n-2].y() ), points[0], points[1] );

            d_eq.resize( n - 1 );
            
            double dq = 0;
            double dr = 0;

            d_eq[1] = eqSpline0;

            double slope1 = ( points[2].y() - points[1].y() ) / d_eq[1].u;

            // a) p1 * b[0] + q1 * b[1] + u1 * b[2] = r1
            // b) p2 * b[n-2] + q2 * b[0] + u2 * b[1] = r2
            // c) pi * b[i-1] + qi * b[i] + ui * b[i+1] = ri
            //
            // Using c) we can substitute b[i] ( starting from 2 ) by b[i+1] 
            // until we reach n-1. As we know, that b[0] == b[n-1] we found 
            // an equation where only 2 coefficients ( for b[n-2], b[0] ) are left unknown.
            // Each step we have an equation that depends on b[0], b[i] and b[i+1] 
            // that can also be used to substitute b[i] in b). Ding so we end up with another
            // equation depending on b[n-2], b[0] only.
            // Finally 2 equations with 2 coefficients can be solved.

            for ( int i = 2; i < n - 1; i++ )
            {
                const Equation3 &eq1 = d_eq[i-1];
                Equation3 &eq2 = d_eq[i];

                dq += eq1.p * eq1.p / eq1.q;
                dr += eq1.p * eq1.r / eq1.q;

                eq2.u = points[i+1].x() - points[i].x();
                const double slope2 = ( points[i+1].y() - points[i].y() ) / eq2.u;

                const double k = eq1.u / eq1.q;

                eq2.p = -eq1.p * k;
                eq2.q = 2.0 * ( eq1.u + eq2.u ) - eq1.u * k;
                eq2.r = 3.0 * ( slope2 - slope1 ) - eq1.r * k;

                slope1 = slope2;
            }


            // b[0] * d_p[n-2] + b[n-2] * d_q[n-2] + b[n-1] * pN = d_r[n-2]
            eqn.setup( d_eq[n-2].q, d_eq[n-2].p + eqSplineN.p , d_eq[n-2].r );

            // b[n-2] * pN + b[0] * ( qN - dq ) + b[n-2] * d_p[n-2] = rN - dr
            eqX.setup( d_eq[n-2].p + eqSplineN.p, eqSplineN.q - dq, eqSplineN.r - dr );
        }

        void resolveSpline( const QPolygonF &points, double b0, double bi )
        {
            const int n = points.size();

            for ( int i = n - 3; i >= 1; i-- )
            {
                const Equation3 &eq = d_eq[i];

                const double b = eq.resolved2( b0, bi );
                d_store.storePrevious( i, eq.u, points[i], points[i+1], b, bi );

                bi = b;
            }
        }

        void resolveSpline2( const QPolygonF &points,
            double b0, double bi, QVector<double> &m )
        {
            const int n = points.size();

            bi = d_eq[0].resolved3( b0, bi );

            for ( int i = 1; i < n - 2; i++ )
            {
                const Equation3 &eq = d_eq[i];

                const double b = eq.resolved3( b0, bi );
                m[i+1] = m[i] + ( b + bi ) * d_eq[i].u;

                bi = b;
            }
        }

        void resolveSpline3( const QPolygonF &points,
            double b0, double b1, QVector<double> &m )
        {
            const int n = points.size();

            double h0 = ( points[1].x() - points[0].x() );
            double s0 = ( points[1].y() - points[0].y() ) / h0;

            m[1] = m[0] + ( b0 + b1 ) * h0;

            for ( int i = 1; i < n - 1; i++ )
            {
                const double h1 = ( points[i+1].x() - points[i].x() );
                const double s1 = ( points[i+1].y() - points[i].y() ) / h1;

                const double r = 3.0 * ( s1 - s0 );

                const double b2 = ( r - h0 * b0 - 2.0 * ( h0 + h1 ) * b1 ) / h1;
                m[i+1] = m[i] + ( b1 + b2 ) * h1;

                h0 = h1;
                s0 = s1;
                b0 = b1;
                b1 = b2;
            }
        }

        void resolveSpline4( const QPolygonF &points,
            double b2, double b1, QVector<double> &m )
        {
            const int n = points.size();

            double h2 = ( points[n-1].x() - points[n-2].x() );
            double s2 = ( points[n-1].y() - points[n-2].y() ) / h2;

            for ( int i = n - 2; i > 1; i-- )
            {
                const double h1 = ( points[i].x() - points[i-1].x() );
                const double s1 = ( points[i].y() - points[i-1].y() ) / h1;

                const double r = 3.0 * ( s2 - s1 );
                const double k = 2.0 * ( h1 + h2 );

                const double b0 = ( r - h2 * b2 - k * b1 ) / h1;

                m[i-1] = m[i] - ( b0 + b1 ) * h1;

                h2 = h1;
                s2 = s1;
                b2 = b1;
                b1 = b0;
            }
        }

    public:
        QVector<Equation3> d_eq;
        T d_store;
    };
}

static void qwtSetupEndEquations( QwtSplineC1::BoundaryCondition type, 
    const QPolygonF &points, double clampedBegin, double clampedEnd, 
    QwtSplineCubicP::Equation3 eq[2] )
{
    const int n = points.size();

    const double h0 = points[1].x() - points[0].x();
    const double s0 = ( points[1].y() - points[0].y() ) / h0;

    const double hn = ( points[n-1].x() - points[n-2].x() );
    const double sn = ( points[n-1].y() - points[n-2].y() ) / hn;

    switch( type )
    {
        case QwtSplineC1::Clamped:
        {
            // first derivative at end points given

            // 3 * a1 * h + b1 = b2
            // a1 * h * h + b1 * h + c1 = s

            // c1 = slopeBegin
            // => b1 * ( 2 * h / 3.0 ) + b2 * ( h / 3.0 ) = s - slopeBegin

            // c2 = slopeEnd
            // => b1 * ( 1.0 / 3.0 ) + b2 * ( 2.0 / 3.0 ) = ( slopeEnd - s ) / h;

            eq[0].setup( 2 * h0 / 3.0, h0 / 3.0, 0.0, s0 - clampedBegin );
            eq[1].setup( 0.0, 1.0 / 3.0 * hn, 2.0 / 3.0 * hn, clampedEnd - sn );

            break;
        }
        case QwtSplineC1::Clamped2:
        {
            // second derivative at end points given

            // b0 = 0.5 * cvStart
            // => b0 * 1.0 + b1 * 0.0 = 0.5 * cvStart

            // b1 = 0.5 * cvEnd
            // => b0 * 0.0 + b1 * 1.0 = 0.5 * cvEnd

            eq[0].setup( 1.0, 0.0, 0.0, 0.5 * clampedBegin ); 
            eq[1].setup( 0.0, 0.0, 1.0, 0.5 * clampedEnd ); 
            break;
        }
        case QwtSplineC1::Clamped3:
        {
            // third derivative at end point given

            // 3 * a * h0 + b[0] = b[1]

            // a = marg_0 / 6.0
            // => b[0] * 1.0 + b[1] * ( -1.0 ) = -0.5 * v0 * h0

            // a = marg_n / 6.0
            // => b[n-2] * 1.0 + b[n-1] * ( -1.0 ) = -0.5 * v1 * h5

            eq[0].setup( 1.0, -1.0, 0.0, -0.5 * clampedBegin * h0 ); 
            eq[1].setup( 0.0, 1.0, -1.0, -0.5 * clampedEnd * hn ); 

            break;
        }
        case QwtSplineCubic::NotAKnot:
        {
            // not-a-node condition

            const double h1 = points[2].x() - points[1].x();
            const double hn1 = ( points[n-2].x() - points[n-3].x() );

            eq[0].setup( 1.0, -( 1.0 + h0 / h1 ), h0 / h1, 0.0 );
            eq[1].setup( hn / hn1, -( 1.0 + hn / hn1 ), 1.0, 0.0 );

            break;
        }
        case QwtSplineCubic::LinearRunout:
        {
            // c1 = s0
            // c2 = sn

#if 0
            // clampedBegin/clampedEnd != 0: TODO ...
#endif
            eq[0].setup( 2 * h0 / 3.0, h0 / 3.0, 0.0, 0.0 );
            eq[1].setup( 0.0, 1.0 / 3.0 * hn, 2.0 / 3.0 * hn, 0.0 );

            break;
        }
        case QwtSplineCubic::ParabolicRunout:
        {
            // parabolic runout
            // b0 = b1 => ( 1.0 ) * b0 + ( -1.0 ) * b1 = 0.0;

            eq[0].setup( 1.0, -1.0, 0.0, 0.0 ); 
            eq[1].setup( 0.0, 1.0, -1.0, 0.0 ); 
            break;
        }
        case QwtSplineCubic::CubicRunout:
        {
            // b0 = 2 * b1 - b2
            // => 1.0 * b0 - 2 * b1 + 1.0 * b2 = 0.0

            eq[0].setup( 1.0, -2.0, 1.0, 0.0 ); 
            eq[1].setup( 1.0, -2.0, 1.0, 0.0 ); 
            break;
        }
        case QwtSplineCubic::Natural:
        default:
        {
            // second derivative at end points set to 0.0
            eq[0].setup( 1.0, 0.0, 0.0, 0.0 ); 
            eq[1].setup( 0.0, 0.0, 1.0, 0.0 ); 
            break;
        }
    }
}

class QwtSplineCubic::PrivateData
{
public:
    PrivateData()
    {
    }
};

QwtSplineCubic::QwtSplineCubic()
{
    d_data = new PrivateData;
    setBoundaryConditions( QwtSplineC1::Natural );
}

QwtSplineCubic::~QwtSplineCubic()
{
    delete d_data;
}

QVector<double> QwtSplineCubic::slopesX( const QPolygonF &points ) const
{
    using namespace QwtSplineCubicP;

    if ( points.size() <= 2 )
        return QVector<double>();

    if ( boundaryCondition() == QwtSplineCubic::NotAKnot )
    {
        const int n = points.size();
        if ( n == 3 )
        {
#if 0
            const double h0 = points[1].x() - points[0].x();
            const double h1 = points[2].x() - points[1].x();

            const double s0 = ( points[1].y() - points[0].y() ) / h0;
            const double s1 = ( points[2].y() - points[1].y() ) / h1;

            /*
              the system is under-determined and we only 
              compute a quadratic spline.                   
             */

            const double b = ( s1 - s0 ) / ( h0 + h1 );

            QVector<double> m( 3 );
            m[0] = s0 - h0 * b;
            m[1] = s1 - h1 * b;
            m[2] = s1 + h1 * b;

            return m;
#else
            return QVector<double>();
#endif
        }
    }

    if ( boundaryCondition() == QwtSplineCubic::Periodic )
    {
        EquationSystem2<SlopeStore> eqs;
        eqs.resolve( points );

        return eqs.store().slopes();
    }

    Equation3 eq[2];
    qwtSetupEndEquations( boundaryCondition(), points, 
        boundaryValueBegin(), boundaryValueEnd(), eq );

    EquationSystem<SlopeStore> eqs;
    eqs.setStartCondition( eq[0].p, eq[0].q, eq[0].u, eq[0].r );
    eqs.setEndCondition( eq[1].p, eq[1].q, eq[1].u, eq[1].r );
    eqs.resolve( points );

    return eqs.store().slopes();
}

QVector<double> QwtSplineCubic::curvaturesX( const QPolygonF &points ) const
{
    using namespace QwtSplineCubicP;

    if ( points.size() <= 2 )
        return QVector<double>();

    if ( points.size() == 3 )
    {
        if ( boundaryCondition() == QwtSplineCubic::NotAKnot )
            return QVector<double>();
    }

    if ( boundaryCondition() == QwtSplineCubic::Periodic )
    {
        EquationSystem2<CurvatureStore> eqs;
        eqs.resolve( points );

        return eqs.store().curvatures();
    }

    Equation3 eq[2];
    qwtSetupEndEquations( boundaryCondition(), points, 
        boundaryValueBegin(), boundaryValueEnd(), eq );

    EquationSystem<CurvatureStore> eqs;
    eqs.setStartCondition( eq[0].p, eq[0].q, eq[0].u, eq[0].r );
    eqs.setEndCondition( eq[1].p, eq[1].q, eq[1].u, eq[1].r );
    eqs.resolve( points );

    return eqs.store().curvatures();
}

