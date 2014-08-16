/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H 1

#include "qwt_global.h"
#include "qwt_spline_polynom.h"
#include <qpolygon.h>
#include <qpainterpath.h>
#include <qmath.h>

class QWT_EXPORT QwtSplineParameter
{
public:
    enum Type
    {
        ParameterX,
        ParameterUniform,
        ParameterCentripetral,
        ParameterChordal,
        ParameterManhattan
    };

    QwtSplineParameter( int type );
    virtual ~QwtSplineParameter();

    int type() const;

    virtual double value( const QPointF &p1, const QPointF &p2 ) const;
    
    static double valueX( const QPointF &p1, const QPointF &p2 );
    static double valueChordal( const QPointF &p1, const QPointF &p2 );
    static double valueUniform( const QPointF &p1, const QPointF &p2 );
    static double valueManhattan( const QPointF &p1, const QPointF &p2 );

    struct param
    {
        param( const QwtSplineParameter * );
        double operator()( const QPointF &p1, const QPointF &p2 ) const;

        const QwtSplineParameter *parameter;
    };

    struct paramX
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

    struct paramUniform
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

    struct paramChordal
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

    struct paramManhattan
    {
        double operator()( const QPointF &p1, const QPointF &p2 ) const;
    };

private:
    const int d_type;
};

class QWT_EXPORT QwtSpline
{
public:
    QwtSpline();
    virtual ~QwtSpline();

    void setParametrization( int type );
    void setParametrization( QwtSplineParameter * );
    const QwtSplineParameter *parametrization() const;

    virtual QPainterPath pathP( const QPolygonF & ) const;
    virtual QPolygonF polygonP( const QPolygonF &, 
        double distance, bool withNodes ) const;

    virtual QVector<QLineF> bezierControlPointsP( const QPolygonF &points ) const = 0;

private:
    QwtSplineParameter *d_parameter;
};

class QWT_EXPORT QwtSplineG1: public QwtSpline
{           
public:     
    QwtSplineG1();
    virtual ~QwtSplineG1();
};

class QWT_EXPORT QwtSplineC1: public QwtSplineG1
{
public:
    enum BoundaryCondition 
    {
        Clamped,
        Clamped2,
        Clamped3,

        Natural,

        LinearRunout,
        ParabolicRunout,
        CubicRunout,

        NotAKnot,
        Periodic
    };

    QwtSplineC1();
    virtual ~QwtSplineC1();

    void setBoundaryConditions( BoundaryCondition );
    BoundaryCondition boundaryCondition() const;

    void setBoundaryValues( double valueBegin, double valueEnd );

    double boundaryValueBegin() const;
    double boundaryValueEnd() const;

    virtual QPainterPath pathP( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlPointsP( const QPolygonF &points ) const;

    virtual QVector<double> slopesX( const QPolygonF & ) const = 0;

    virtual QPolygonF polygonX( int numPoints, const QPolygonF & ) const;
    virtual QVector<QwtSplinePolynom> polynomsX( const QPolygonF & ) const;

//protected:
    virtual double slopeBegin( const QPolygonF &points, double m1, double m2 ) const;
    virtual double slopeEnd( const QPolygonF &points, double m1, double m2 ) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

class QWT_EXPORT QwtSplineC2: public QwtSplineC1
{
public:
    QwtSplineC2();
    virtual ~QwtSplineC2();

    virtual QVector<double> curvaturesX( const QPolygonF & ) const = 0;
};

inline double QwtSplineParameter::valueX( 
    const QPointF &p1, const QPointF &p2 ) 
{
    return p2.x() - p1.x();
}

inline double QwtSplineParameter::valueUniform(
    const QPointF &p1, const QPointF &p2 )
{
    Q_UNUSED( p1 )
    Q_UNUSED( p2 )

    return 1.0;
}

inline double QwtSplineParameter::valueChordal( 
    const QPointF &p1, const QPointF &p2 ) 
{
    const double dx = p1.x() - p2.x();
    const double dy = p1.y() - p2.y();

    return qSqrt( dx * dx + dy * dy );
}

inline double QwtSplineParameter::valueManhattan(
    const QPointF &p1, const QPointF &p2 )
{
    return qAbs( p2.x() - p1.x() ) + qAbs( p2.y() - p1.y() );
}

inline QwtSplineParameter::param::param( const QwtSplineParameter *p ):
    parameter( p ) 
{
}
    
inline double QwtSplineParameter::param::operator()( 
    const QPointF &p1, const QPointF &p2 ) const
{
    return parameter->value( p1, p2 );
}

inline double QwtSplineParameter::paramX::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParameter::valueX( p1, p2 );
}

inline double QwtSplineParameter::paramUniform::operator()(
    const QPointF &p1, const QPointF &p2 ) const
{
    return QwtSplineParameter::valueUniform( p1, p2 );
}
    
inline double QwtSplineParameter::paramChordal::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParameter::valueChordal( p1, p2 );
}

inline double QwtSplineParameter::paramManhattan::operator()( 
    const QPointF &p1, const QPointF &p2 ) const 
{
    return QwtSplineParameter::valueManhattan( p1, p2 );
}

#endif
