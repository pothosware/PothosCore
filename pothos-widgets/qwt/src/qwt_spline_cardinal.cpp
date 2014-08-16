/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_cardinal.h"

namespace QwtSplineCardinalG1P
{
    class PathStore
    {
    public:
        inline void init( int )
        {
        }

        inline void start( const QPointF &p0 )
        {
            path.moveTo( p0 );
        }

        inline void addCubic( const QPointF &cp1,
            const QPointF &cp2, const QPointF &p2 )
        {
            path.cubicTo( cp1, cp2, p2 );
        }

        QPainterPath path;
    };

    class ControlPointsStore
    {
    public:
        inline ControlPointsStore():
            d_cp( NULL )
        {
        }

        inline void init( int size )
        {
            controlPoints.resize( size );
            d_cp = controlPoints.data();
        }

        inline void start( const QPointF & )
        {
        }

        inline void addCubic( const QPointF &cp1,
            const QPointF &cp2, const QPointF & )
        {
            QLineF &l = *d_cp++;
            l.setPoints( cp1, cp2 );
        }

        QVector<QLineF> controlPoints;

    private:
        QLineF* d_cp;
    };
}

static inline QwtSplineCardinalG1::Tension qwtTension(
    double d13, double d23, double d24,
    const QPointF &p1, const QPointF &p2,
    const QPointF &p3, const QPointF &p4 )
{
    const bool b1 = ( d13 / 3.0 ) < d23;
    const bool b2 = ( d24 / 3.0 ) < d23;

    QwtSplineCardinalG1::Tension tension;
    if ( b1 & b2 )
    {
        tension.t1 = ( p1 != p2 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
        tension.t2 = ( p3 != p4 ) ? ( 1.0 / 3.0 ) : ( 2.0 / 3.0 );
    }
    else
    {
        tension.t1 = d23 / ( b1 ? d24 : d13 );
        tension.t2 = d23 / ( b2 ? d13 : d24 );
    }

    return tension;
}

template< class Param >
static inline QVector<QwtSplineCardinalG1::Tension> qwtTensions( 
    const QPolygonF &points, Param param ) 
{
    const int size = points.size();
    QVector<QwtSplineCardinalG1::Tension> tensions2( size - 1 );

    const QPointF *p = points.constData();
    QwtSplineCardinalG1::Tension *t = tensions2.data();

    double d13 = param(p[0], p[2]);

    const double d0 = param(p[0], p[1]);
    t[0] = qwtTension( d0, d0, d13, p[0], p[0], p[1], p[2] );

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d23 = param( p[i], p[i+1] );
        const double d24 = param( p[i], p[i+2] );

        t[i] = qwtTension( d13, d23, d24, p[i-1], p[i], p[i+1], p[i+2] );

        d13 = d24;
    }

    const double dn = param(p[size - 2], p[size - 1]);

    t[size-2] = qwtTension( d13, dn, dn,
        p[size - 3], p[size - 2], p[size - 1], p[size - 1] );

    return tensions2;
}

template< class SplineStore >
static SplineStore qwtSplinePathG1( 
    const QwtSplineCardinalG1 *spline, const QPolygonF &points ) 
{
    const int size = points.size();

    const QVector<QwtSplineCardinalG1::Tension> tensions2 = spline->tensions( points );
    if ( tensions2.size() != size - 1 )
        return SplineStore();

    const QPointF *p = points.constData();
    const QwtSplineCardinalG1::Tension *t = tensions2.constData();

    SplineStore store;
    store.init( size - 1 );
    store.start( p[0] );
    
    QPointF vec1 = ( p[1] - p[0] ) * 0.5;
    for ( int i = 0; i < size - 2; i++ )
    {
        const QPointF vec2 = ( p[i+2] - p[i] ) * 0.5;
        store.addCubic( p[i] + vec1 * t[i].t1, 
            p[i+1] - vec2 * t[i].t2, p[i+1] );
        
        vec1 = vec2;
    }   
    
    const QPointF vec2 = 0.5 * ( p[size - 1] - p[size - 2] );

    store.addCubic( p[size - 2] + vec1 * t[size-2].t1,
        p[size - 1] - vec2 * t[size-2].t2, p[size - 1] );

    return store;
}

template< class SplineStore, class Param >
static SplineStore qwtSplinePathPleasing( const QPolygonF &points, Param param )
{
    const int size = points.size();

    const QPointF *p = points.constData();

    SplineStore store;
    store.init( size - 1 );
    store.start( p[0] );

    const double d0 = param(p[0], p[1]);
    double d13 = param(p[0], p[2]);

    const QwtSplineCardinalG1::Tension t0 = 
        qwtTension( d0, d0, d13, p[0], p[0], p[1], p[2] );

    const QPointF vec0 = ( p[1] - p[0] ) * 0.5;
    QPointF vec1 = ( p[2] - p[0] ) * 0.5;

    store.addCubic( p[0] + vec0 * t0.t1, p[1] - vec1 * t0.t2, p[1] );

    for ( int i = 1; i < size - 2; i++ )
    {
        const double d23 = param(p[i], p[i+1]);
        const double d24 = param(p[i], p[i+2]);
        const QPointF vec2 = ( p[i+2] - p[i] ) * 0.5;

        const QwtSplineCardinalG1::Tension t =
            qwtTension( d13, d23, d24, p[i-1], p[i], p[i+1], p[i+2] );

        store.addCubic( p[i] + vec1 * t.t1, p[i+1] - vec2 * t.t2, p[i+1] );

        d13 = d24;
        vec1 = vec2;
    }

    const double d23 = param(p[size-2], p[size-1]);
    const QPointF vec2 = 0.5 * ( p[size-1] - p[size-2] );

    const QwtSplineCardinalG1::Tension tn =
        qwtTension( d13, d23, d23, p[size-3], p[size-2], p[size-1], p[size-1] );

    store.addCubic( p[size-2] + vec1 * tn.t1, p[size-1] - vec2 * tn.t2, p[size-1] );

    return store;
}

QwtSplineCardinalG1::QwtSplineCardinalG1()
{
}

QwtSplineCardinalG1::~QwtSplineCardinalG1()
{
}

QVector<QLineF> QwtSplineCardinalG1::bezierControlPointsP( 
    const QPolygonF &points ) const
{
    using namespace QwtSplineCardinalG1P;
    const ControlPointsStore store = 
        qwtSplinePathG1<ControlPointsStore>( this, points );

    return store.controlPoints;
}

QPainterPath QwtSplineCardinalG1::pathP( const QPolygonF &points ) const
{
    using namespace QwtSplineCardinalG1P;
    const PathStore store = qwtSplinePathG1<PathStore>( this, points );

    return store.path;
}

QwtSplinePleasing::QwtSplinePleasing()
{
}

QwtSplinePleasing::~QwtSplinePleasing()
{
}

QPainterPath QwtSplinePleasing::pathP( const QPolygonF &points ) const
{
    const int size = points.size();
    if ( size <= 2 )
        return QwtSplineCardinalG1::pathP( points );

    using namespace QwtSplineCardinalG1P;

    PathStore store;
    switch( parametrization()->type() )
    {
        case QwtSplineParameter::ParameterX:
        {
            store = qwtSplinePathPleasing<PathStore>( 
                points, QwtSplineParameter::paramX() );
            break;
        }
        case QwtSplineParameter::ParameterUniform:
        {
            store = qwtSplinePathPleasing<PathStore>( 
                points, QwtSplineParameter::paramUniform() );
            break;
        }
        case QwtSplineParameter::ParameterChordal:
        {
            store = qwtSplinePathPleasing<PathStore>( points, 
                QwtSplineParameter::paramChordal() );
            break;
        }
        default:
        {
            store = qwtSplinePathPleasing<PathStore>( points, 
                QwtSplineParameter::param( parametrization() ) );
        }
    }

    return store.path;
}

QVector<QLineF> QwtSplinePleasing::bezierControlPointsP( 
    const QPolygonF &points ) const
{
    return QwtSplineCardinalG1::bezierControlPointsP( points );
}

QVector<QwtSplineCardinalG1::Tension> 
QwtSplinePleasing::tensions( const QPolygonF &points ) const
{
    QVector<Tension> tensions2;

    if ( points.size() <= 2 )
        return tensions2;

    switch( parametrization()->type() )
    {
        case QwtSplineParameter::ParameterX:
        {
            tensions2 = qwtTensions( points, QwtSplineParameter::paramX() );
            break;
        }
        case QwtSplineParameter::ParameterUniform:
        {
            tensions2 = qwtTensions( points, QwtSplineParameter::paramUniform() );
            break;
        }
        case QwtSplineParameter::ParameterChordal:
        {
            tensions2 = qwtTensions( points, QwtSplineParameter::paramChordal() );
            break;
        }
        default:
        {
            tensions2 = qwtTensions( points, QwtSplineParameter::param( parametrization() ) );
        }
    }

    return tensions2;
}
