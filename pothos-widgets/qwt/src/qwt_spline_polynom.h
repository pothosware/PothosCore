/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_POLYNOM_H
#define QWT_SPLINE_POLYNOM_H 1

#include "qwt_global.h"
#include <qpoint.h>
#include <qmetatype.h>

#ifndef QT_NO_DEBUG_STREAM
#include <qdebug.h>
#endif

class QWT_EXPORT QwtSplinePolynom
{
public:
    QwtSplinePolynom( double a = 0.0, double b = 0.0, double c = 0.0 );

    double value( double x ) const;
    double slope( double x ) const;
    double curvature( double x ) const;

    static QwtSplinePolynom fromSlopes( 
        const QPointF &p1, double m1, 
        const QPointF &p2, double m2 );

    static QwtSplinePolynom fromSlopes( 
        double x, double y, double m1, double m2 );

    static QwtSplinePolynom fromCurvatures( 
        const QPointF &p1, double cv1,
        const QPointF &p2, double cv2 );
    
    static QwtSplinePolynom fromCurvatures(
        double dx, double dy, double cv1, double cv2 );

public:
    double c1;
    double c2;
    double c3;
};

inline QwtSplinePolynom::QwtSplinePolynom( double a, double b, double c ):
    c1(c),
    c2(b),
    c3(a)
{
}

inline double QwtSplinePolynom::value( double x ) const
{
    return ( ( ( c3 * x ) + c2 ) * x + c1 ) * x;
}   

inline double QwtSplinePolynom::slope( double x ) const
{   
    return ( 3.0 * c3 * x + 2.0 * c2 ) * x + c1;
}

inline double QwtSplinePolynom::curvature( double x ) const
{   
    return 6.0 * c3 * x + 2.0 * c2;
}

inline QwtSplinePolynom QwtSplinePolynom::fromSlopes( 
    const QPointF &p1, double m1, const QPointF &p2, double m2 )
{
    return fromSlopes( p2.x() - p1.x(), p2.y() - p1.y(), m1, m2 );
}

inline QwtSplinePolynom QwtSplinePolynom::fromSlopes( 
    double x, double y, double m1, double m2 )
{
    const double c = m1;
    const double b = ( 3.0 * y / x - 2 * m1 - m2 ) / x;
    const double a = ( ( m2 - m1 ) / x - 2.0 * b ) / ( 3.0 * x );

    return QwtSplinePolynom( a, b, c );
}

inline QwtSplinePolynom QwtSplinePolynom::fromCurvatures( 
    const QPointF &p1, double cv1, const QPointF &p2, double cv2 )
{
    return fromCurvatures( p2.x() - p1.x(), p2.y() - p1.y(), cv1, cv2 );
}

inline QwtSplinePolynom QwtSplinePolynom::fromCurvatures( 
    double x, double y, double cv1, double cv2 )
{
    const double a = ( cv2 - cv1 ) / ( 6.0 * x );
    const double b = 0.5 * cv1;
    const double c = y / x - ( a * x + b ) * x;

    return QwtSplinePolynom( a, b, c );
}

Q_DECLARE_METATYPE( QwtSplinePolynom )

#ifndef QT_NO_DEBUG_STREAM
inline QDebug operator<<( QDebug debug, const QwtSplinePolynom &p )
{
    debug.nospace() << "Polynom(" << p.c3 << ", " << p.c2 << ", " << p.c1 << ")";
    return debug.space();
}
#endif

#endif
