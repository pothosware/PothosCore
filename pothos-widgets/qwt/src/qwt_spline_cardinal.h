/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_CARDINAL_H
#define QWT_SPLINE_CARDINAL_H 1

#include "qwt_spline.h"

class QWT_EXPORT QwtSplineCardinalG1: public QwtSplineG1
{
public:
    class Tension
    {
    public:
        Tension()
        {
        }

        Tension( double s1, double s2 ):
            t1( s1 ),
            t2( s2 )
        {
        }

        double t1;
        double t2;
    };

    QwtSplineCardinalG1();
    virtual ~QwtSplineCardinalG1();

    virtual QVector<QLineF> bezierControlPointsP( const QPolygonF &points ) const;
    virtual QPainterPath pathP( const QPolygonF &points ) const;

    virtual QVector<Tension> tensions( const QPolygonF & ) const = 0;
};

class QWT_EXPORT QwtSplinePleasing: public QwtSplineCardinalG1
{
public:
    QwtSplinePleasing();
    virtual ~QwtSplinePleasing();
    
    virtual QPainterPath pathP( const QPolygonF & ) const;
    virtual QVector<QLineF> bezierControlPointsP( const QPolygonF &points ) const;

    virtual QVector<Tension> tensions( const QPolygonF & ) const;
};

#endif
