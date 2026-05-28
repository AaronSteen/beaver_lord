#if !defined(BEAVER_MATH_H)

union vector2
{
    struct
    {
        real32 X; 
        real32 Y;
    };
    real32 Dimensions[2];
};

vector2
NewVector(real32 X, real32 Y)
{
    vector2 Result;

    Result.X = X;
    Result.Y = Y;

    return(Result);
}

vector2 
operator*(real32 Scalar, vector2 Vector)
{
    vector2 Result;
    
    Result.X = Vector.X * Scalar;
    Result.Y = Vector.Y * Scalar;

    return(Result);
}

vector2 &
operator*=(vector2 &Vector, real32 Scalar)
{
    Vector = Scalar * Vector;

    return(Vector);
}

vector2 
operator-(vector2 A)
{
    vector2 Result;

    Result.X = -A.X;
    Result.Y = -A.Y;

    return(Result);
}

vector2 
operator+(vector2 A, vector2 B)
{
    vector2 Result;

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;

    return(Result);
}

vector2 &
operator+=(vector2 &A, vector2 B)
{
    A = A + B;

    return(A);
}

vector2 
operator-(vector2 A, vector2 B)
{
    vector2 Result;

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;

    return(Result);
}

#define BEAVER_MATH_H
#endif
