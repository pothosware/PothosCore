/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_local.h"
#include <qmath.h>

static inline double qwtSlopeP( const QPointF &p1, const QPointF &p2 )
{
    // ???
    const double dx = p2.x() - p1.x();
    return dx ? ( p2.y() - p1.y() ) / dx : 0.0;
}

static inline void qwtCubicToP( const QPointF &p1, double m1,
    const QPointF &p2, double m2, QPainterPath &path )
{
    const double dx3 = ( p2.x() - p1.x() ) / 3.0;

    path.cubicTo( p1.x() + dx3, p1.y() + m1 * dx3,
        p2.x() - dx3, p2.y() - m2 * dx3,
        p2.x(), p2.y() );
}

namespace QwtSplineLocalP
{
    class PathStore
    {
    public:
        inline void init( const QPolygonF & )
        {
        }

        inline void start( const QPointF &p0, double )
        {
            path.moveTo( p0 );
        }

        inline void addCubic( const QPointF &p1, double m1,
            const QPointF &p2, double m2 )
        {
            qwtCubicToP( p1, m1, p2, m2, path );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
    public:
        inline void init( const QPolygonF &points )
        {
            if ( points.size() > 0 )
                controlPoints.resize( points.size() - 1 );
            d_cp = controlPoints.data();
        }

        inline void start( const QPointF &, double )
        {
        }

        inline void addCubic( const QPointF &p1, double m1,
            const QPointF &p2, double m2 )
        {
            const double dx3 = ( p2.x() - p1.x() ) / 3.0;

            QLineF &l = *d_cp++;
            l.setLine( p1.x() + dx3, p1.y() + m1 * dx3, 
                p2.x() - dx3, p2.y() - m2 * dx3 );
        }

        QVector<QLineF> controlPoints;

    private:
        QLineF *d_cp;
    };

    class SlopeStore
    {
    public:
        void init( const QPolygonF &points )
        {
            slopes.resize( points.size() );
            d_m = slopes.data();
        }

        inline void start( const QPointF &, double m0 )
        {   
            *d_m++ = m0;
        }

        inline void addCubic( const QPointF &, double,
            const QPointF &, double m2 )
        {
            *d_m++ = m2;
        }

        QVector<double> slopes;

    private:
        double *d_m;
    };
};

static inline double qwtAkima( double s1, double s2, double s3, double s4 )
{
    if ( ( s1 == s2 ) && ( s3 == s4 ) )
    {
        return 0.5 * ( s2 + s3 );
    }

    const double ds12 = qAbs( s2 - s1 );
    const double ds34 = qAbs( s4 - s3 );

    return ( s2 * ds34 + s3 * ds12 ) / ( ds12 + ds34 );
}

static inline double qwtHarmonicMean( 
    double dx1, double dy1, double dx2, double dy2 )
{
    if ( ( dy1 > 0.0 ) == ( dy2 > 0.0 ) )
    {
        if ( ( dy1 != 0.0 ) && ( dy2 != 0.0 ) )
            return 2.0 / ( dx1 / dy1 + dx2 / dy2 );
    }

    return 0.0;
}

static inline double qwtHarmonicMean( double s1, double s2 )
{
    if ( ( s1 > 0.0 ) == ( s2 > 0.0 ) )
    {
        if ( ( s1 != 0.0 ) && ( s2 != 0.0 ) )
            return 2.0 / ( 1.0 / s1 + 1.0 / s2 );
    }

    return 0.0;
}

static inline void qwtSplineCardinalBoundaries( 
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    const double m2 = qwtSlopeP( p[0], p[2] );

    if ( n == 3 )
    {
        const double s0 = qwtSlopeP( p[0], p[1] );
        slopeEnd = spline->slopeEnd( points, s0, m2 );
        slopeBegin = spline->slopeBegin( points, m2, slopeEnd );
    }
    else
    {
        const double m3 = qwtSlopeP( p[1], p[3] );
        slopeBegin = spline->slopeBegin( points, m2, m3 );

        const double mn1 = qwtSlopeP( p[n-4], p[n-2] );
        const double mn2 = qwtSlopeP( p[n-3], p[n-1] );

        slopeEnd = spline->slopeEnd( points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplinePathCardinal( 
    const QwtSplineLocal *spline, const QPolygonF &points )
{
    double slopeBegin, slopeEnd;
    qwtSplineCardinalBoundaries( spline, points, slopeBegin, slopeEnd );

    const double s = 1.0 - spline->tension();

    const QPointF *p = points.constData();
    const int size = points.size();

    SplineStore store;
    store.init( points );
    store.start( p[0], s * slopeBegin );

    double m1 = s * slopeBegin;
    for ( int i = 1; i < size - 1; i++ )
    {
        const double m2 = s * qwtSlopeP( p[i-1], p[i+1] );
        store.addCubic( p[i-1], m1, p[i], m2 );

        m1 = m2;
    }

    store.addCubic( p[size-2], s * m1, p[size-1], s * slopeEnd );

    return store;
}

static inline void qwtSplineAkimaBoundaries(
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    if ( spline->boundaryCondition() == QwtSplineC1::Clamped )
    {
        slopeBegin = spline->boundaryValueBegin();
        slopeEnd = spline->boundaryValueEnd();

        return;
    }

    const int n = points.size();
    const QPointF *p = points.constData();

    if ( n == 3 )
    {
        const double s1 = qwtSlopeP( p[0], p[1] );
        const double s2 = qwtSlopeP( p[1], p[2] );
        const double m = qwtAkima( s1, s1, s2, s2 );

        slopeBegin = spline->slopeBegin( points, m, s2 );
        slopeEnd = spline->slopeEnd( points, slopeBegin, m );
    }
    else if ( n == 4 )
    {
        const double s1 = qwtSlopeP( p[0], p[1] );
        const double s2 = qwtSlopeP( p[1], p[2] );
        const double s3 = qwtSlopeP( p[2], p[3] );

        const double m2 = qwtAkima( s1, s1, s2, s2 );
        const double m3 = qwtAkima( s2, s2, s3, s3 );

        slopeBegin = spline->slopeBegin( points, m2, m3 );
        slopeEnd = spline->slopeEnd( points, m2, m3 );
    }
    else
    {
        double s[4];

        for ( int i = 0; i < 4; i++ )
            s[i] = qwtSlopeP( p[i], p[i+1] );

        const double m2 = qwtAkima( s[0], s[0], s[1], s[1] );
        const double m3 = qwtAkima( s[0], s[1], s[2], s[3] );
        slopeBegin = spline->slopeBegin( points, m2, m3 );

        for ( int i = n - 5; i < n - 1; i++ )
            s[i] = qwtSlopeP( p[i], p[i+1] );

        const double mn1 = qwtAkima( s[0], s[1], s[2], s[3] );
        const double mn2 = qwtAkima( s[2], s[2], s[3], s[3] );

        slopeEnd = spline->slopeEnd( points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplinePathAkima( 
    const QwtSplineLocal *spline, const QPolygonF &points )
{
    double slopeBegin, slopeEnd;
    qwtSplineAkimaBoundaries( spline, points, slopeBegin, slopeEnd );

    const double s = 1.0 - spline->tension();

    const int size = points.size();
    const QPointF *p = points.constData();

    SplineStore store;
    store.init( points );
    store.start( p[0], s * slopeBegin );

    double s1 = slopeBegin;
    double s2 = qwtSlopeP( p[0], p[1] );
    double s3 = qwtSlopeP( p[1], p[2] );

    double m1 = s * slopeBegin;

    for ( int i = 0; i < size - 3; i++ )
    {
        const double s4 = qwtSlopeP( p[i+2],  p[i+3] );

        const double m2 = s * qwtAkima( s1, s2, s3, s4 );
        store.addCubic( p[i], m1, p[i+1], m2 );

        s1 = s2;
        s2 = s3;
        s3 = s4;

        m1 = m2;
    }

    const double m2 = s * qwtAkima( s1, s2, s3, slopeEnd );

    store.addCubic( p[size - 3], m1, p[size - 2], m2 );
    store.addCubic( p[size - 2], m2, p[size - 1], slopeEnd );

    return store;
}

static inline void qwtSplineHarmonicMeanBoundaries(
    const QwtSplineLocal *spline, const QPolygonF &points,
    double &slopeBegin, double &slopeEnd )
{
    const int n = points.size();
    const QPointF *p = points.constData();

    const double s1 = qwtSlopeP( p[0], p[1] );
    const double s2 = qwtSlopeP( p[1], p[2] );

    if ( n == 3 )
    {
        const double m = qwtHarmonicMean( s1, s2 );
        slopeBegin = spline->slopeBegin( points, m, s2 );
        slopeEnd = spline->slopeEnd( points, slopeBegin, m );
    }
    else
    {
        const double s3 = qwtSlopeP( p[2], p[3] );

        const double m2 = qwtHarmonicMean( s1, s2 );
        const double m3 = qwtHarmonicMean( s2, s3 );

        const double sn1 = qwtSlopeP( p[n-4], p[n-3] );
        const double sn2 = qwtSlopeP( p[n-3], p[n-2] );
        const double sn3 = qwtSlopeP( p[n-2], p[n-1] );

        const double mn1 = qwtHarmonicMean( sn1, sn2 );
        const double mn2 = qwtHarmonicMean( sn2, sn3 );

        slopeBegin = spline->slopeBegin( points, m2, m3 );
        slopeEnd = spline->slopeEnd( points, mn1, mn2 );
    }
}

template< class SplineStore >
static inline SplineStore qwtSplinePathHarmonicMean( 
    const QwtSplineLocal *spline, const QPolygonF &points )
{
    double slopeBegin, slopeEnd;
    qwtSplineHarmonicMeanBoundaries( spline, points, slopeBegin, slopeEnd );

    const double s = 1.0 - spline->tension();

    const int size = points.size();
    const QPointF *p = points.constData();

    SplineStore store;
    store.init( points );
    store.start( p[0], s * slopeBegin );

    double dx1 = p[1].x() - p[0].x();
    double dy1 = p[1].y() - p[0].y();

    double m1 = s * slopeBegin;
    for ( int i = 1; i < size - 1; i++ )
    {
        const double dx2 = p[i+1].x() - p[i].x();
        const double dy2 = p[i+1].y() - p[i].y();

        const double m2 = s * qwtHarmonicMean( dx1, dy1, dx2, dy2 );

        store.addCubic( p[i-1], m1, p[i], m2 );

        dx1 = dx2;
        dy1 = dy2;
        m1 = m2;
    }

    store.addCubic( p[size - 2], m1, p[size - 1], s * slopeEnd );

    return store; 
}

template< class SplineStore >
static inline SplineStore qwtSplinePathLocal( 
    const QwtSplineLocal *spline, const QPolygonF &points )
{   
    SplineStore store;

    const int size = points.size();
    if ( size <= 1 )
        return store;

    if ( size == 2 )
    {
        const double s0 = qwtSlopeP( points[0], points[1] );
        const double m1 = spline->slopeBegin( points, s0, s0 ) * spline->tension();
        const double m2 = spline->slopeEnd( points, s0, s0 ) * spline->tension();

        store.init( points );
        store.start( points[0], m1 );
        store.addCubic( points[0], m1, points[1], m2 );

        return store;
    }

    switch( spline->type() )
    {   
        case QwtSplineLocal::Cardinal:
        {   
            store = qwtSplinePathCardinal<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::ParabolicBlending:
        {   
            // Bessel
            break;
        }
        case QwtSplineLocal::Akima:
        {   
            store = qwtSplinePathAkima<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::HarmonicMean:
        {   
            store = qwtSplinePathHarmonicMean<SplineStore>( spline, points );
            break;
        }
        case QwtSplineLocal::PChip:
        {   
            break;
        }
        default:
            break;
    }

    return store;
}

QwtSplineLocal::QwtSplineLocal( Type type, double tension ):
    d_type( type ),
    d_tension( 0.0 )
{
    setTension( tension );
    setBoundaryConditions( QwtSplineLocal::LinearRunout );
    setBoundaryValues( 0.0, 0.0 );
}

QwtSplineLocal::~QwtSplineLocal()
{
}

QwtSplineLocal::Type QwtSplineLocal::type() const
{
    return d_type;
}

void QwtSplineLocal::setTension( double tension )
{
    d_tension = qBound( 0.0, tension, 1.0 );
}

double QwtSplineLocal::tension() const
{
    return d_tension;
}

QPainterPath QwtSplineLocal::pathP( const QPolygonF &points ) const
{
    if ( parametrization()->type() == QwtSplineParameter::ParameterX )
        return pathX( points );

    return QwtSplineC1::pathP( points );
}

QVector<QLineF> QwtSplineLocal::bezierControlPointsP( const QPolygonF &points ) const
{
    if ( parametrization()->type() == QwtSplineParameter::ParameterX )
    {   
        return bezierControlPointsX( points );
    }

    return QwtSplineC1::bezierControlPointsP( points );
}

QPainterPath QwtSplineLocal::pathX( const QPolygonF &points ) const
{
    using namespace QwtSplineLocalP;
    return qwtSplinePathLocal<PathStore>( this, points).path;
}
    
QVector<QLineF> QwtSplineLocal::bezierControlPointsX( const QPolygonF &points ) const
{
    using namespace QwtSplineLocalP;
    return qwtSplinePathLocal<ControlPointsStore>( this, points ).controlPoints;
}

QVector<double> QwtSplineLocal::slopesX( const QPolygonF &points ) const
{
    using namespace QwtSplineLocalP;
    return qwtSplinePathLocal<SlopeStore>( this, points ).slopes;
}
