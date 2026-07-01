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

vector2 
operator*(vector2 Vector, real32 Scalar)
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

real32
Square(real32 A)
{
    real32 Result = A*A;

    return(Result);
}

real32
Inner(vector2 A, vector2 B)
{
    real32 Result = (A.X * B.X) + (A.Y * B.Y);

    return(Result);
}

u32
LerpWithUInts(u32 A, u32 B, real32 T)
{
    u32 ToReturn = A * (1 - T) + (T * B);
    return(ToReturn);
}

real32
LerpWithReal32s(real32 A, real32 B, real32 T)
{
    real32 ToReturn = A * (1.0f - T) + (T * B);
    return(ToReturn);
}

vector2
GetVector2FromPolarCoordinates(real32 Argument, real32 Modulus)
{
    real32 X = Modulus * Cos(Argument);
    real32 Y = Modulus * Sin(Argument);

    vector2 Result = {X, Y};

    return(Result);
}

#define BEAVER_MATH_H
#endif
