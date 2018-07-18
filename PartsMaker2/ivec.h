#pragma once

#include <iostream>

namespace IntVec {

typedef float Real;

template <typename Int> struct Int2;
template <typename Int> struct Int3;
template <typename Int> struct Int4;

template <typename Int>
struct Int2
{
    union {
		struct {
			Int x,y;        // standard names for components
		};
		struct {
			Int s,t;
		};
        Int vec_array[2];     // array access
    };

	Int2() { }
	Int2(const Int v) : x(v), y(v) {}
    Int2(const Int _x, const Int _y) : x(_x), y(_y) { }
    Int2(const Int* xy) : x(xy[0]), y(xy[1]) { }
	Int2(const Int2& u) : x(u.x), y(u.y) { }
	Int2(const Int2& u1, const Int2& u2)
		: x(u2.x - u1.x), y(u2.y - u1.y) {}

	inline void set(const Int val)
	{
		x = val;
		y = val;
	}

	inline void set(const Int _x, const Int _y)
	{
		x = _x;
		y = _y;
	}

	inline void set(const Int2 &v)
	{
		x = v.x;
		y = v.y;
	}

	inline void add(const Int2 &u)
	{
		x += u.x;
		y += u.y;
	}

	inline void sub(const Int2 &u)
	{
		x -= u.x;
		y -= u.y;
	}

	inline void scale(const Real val)
	{
		x = (Int)(x*val);
		y = (Int)(y*val);
	}

	inline void lerp(const Real t, const Int2 & u, const Int2 & v)
	{
		x = (Int)(u.x + t * (v.x - u.x));
		y = (Int)(u.y + t * (v.y - u.y));
	}

	inline Int2& operator=(const Int2 & u)
	{
		x = u.x;
		y = u.y;
        return *this;
	}

    inline bool operator==(const Int2 & u) const
    {
        return (u.x == x && u.y == y);
    }

    inline bool operator!=( const Int2& rhs ) const
    {
        return !(*this == rhs );
    }

    inline Int2 & operator*=(const Real lambda)
    {
        x = Int(x*lambda);
		y = Int(y*lambda);
        return *this;
    }

	inline Int2 & operator/=(const Real lambda)
    {
        x = Int(x/lambda);
        y = Int(y/lambda);
        return *this;
    }

    inline Int2 operator - () const
	{
		return Int2(-x, -y);
	}

	inline Int2 & operator-=(const Int2 & u)
    {
        x -= u.x;
        y -= u.y;
        return *this;
    }

    inline Int2 & operator+=(const Int2 & u)
    {
        x += u.x;
        y += u.y;
        return *this;
    }

    inline Int & operator[](const int i)
    {
        return vec_array[i];
    }

    inline const Int operator[](const int i) const
    {
        return vec_array[i];
    }

	inline operator const Int *() const { return vec_array; }

	inline friend Int dot(const Int2 & u, const Int2 & v)
	{
		return u.x*v.x + u.y*v.y;
	}

	inline friend Int2 & madd(Int2 & u, const Int2& v, Real lambda)
	{
	    u.x += Int(v.x*lambda);
	    u.y += Int(v.y*lambda);
		return u;
	}

	inline friend Int2 & mult(Int2 & u, const Int2& v, Real lambda)
	{
	    u.x = Int(v.x*lambda);
	    u.y = Int(v.y*lambda);
		return u;
	}

	inline friend Int2 & mult(Int2 & u, Real lambda, const Int2& v)
	{
	    u.x = Int(v.x*lambda);
	    u.y = Int(v.y*lambda);
		return u;
	}

	inline friend Int2 & mult(Int2 & u, const Int2& v, const Int2& w)
	{
	    u.x = v.x*w.x;
	    u.y = v.y*w.y;
		return u;
	}

	inline friend Int2 & sub(Int2 & u, const Int2& v, const Int2& w)
	{
	    u.x = v.x - w.x;
	    u.y = v.y - w.y;
		return u;
	}

	inline friend Int2 & add(Int2 & u, const Int2& v, const Int2& w)
	{
	    u.x = v.x + w.x;
	    u.y = v.y + w.y;
		return u;
	}

	inline friend Int2 & scale(Int2& u, Real s)
	{
	    u.x = Int(u.x*s);
	    u.y = Int(u.y*s);
		return u;
	}

	inline friend Int2 & scale(Real s, Int2& u)
	{
	    u.x = Int(u.x*s);
	    u.y = Int(u.y*s);
		return u;
	}

	inline friend Int2 & lerp(Int2 & w, Real t, const Int2 & u, const Int2 & v)
	{
		w.lerp(t, u, v);
		return w;
	}

	inline friend const Int2 operator+(const Int2& u, const Int2& v)
	{
		return Int2(u.x + v.x, u.y + v.y);
	}

	inline friend const Int2 operator-(const Int2& u, const Int2& v)
	{
	    return Int2(u.x - v.x, u.y - v.y);
	}

	inline friend const Int2 operator*(Real s, const Int2& u)
	{
		return Int2(Int(s * u.x), Int(s * u.y));
	}

	inline friend const Int2 operator*(const Int2& u, Real s)
	{
		return Int2(Int(s * u.x), Int(s * u.y));
	}

	inline friend const Int2 operator/(const Int2& u, Real s)
	{
		return Int2((Int)(u.x / s), (Int)(u.y / s));
	}

	inline friend Int operator*(const Int2& u, const Int2& v)
	{
		return u.x * v.x + u.y * v.y;
	}

	inline friend std::ostream& operator<<(std::ostream &o, const Int2& u)
	{
		return o << "(" << (int)u.x << "," << (int)u.y << ")";
	}

};

template <typename Int>
struct Int3
{
    union {
		struct {
			Int x,y,z;        // standard names for components
		};
		struct {
			Int r,g,b;
		};
        Int vec_array[3];     // array access
    };

	Int3() { }
	Int3(const Int v) : x(v), y(v), z(v) {}
    Int3(const Int _x, const Int _y, const Int _z)
		: x(_x), y(_y), z(_z) { }
    Int3(const Int* xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]) { }
	Int3(const Int3& u) : x(u.x), y(u.y), z(u.z) { }
	Int3(const Int3& u1, const Int3& u2)
		: x(u2.x - u1.x), y(u2.y - u1.y), z(u2.z - u1.z) {}

	inline void set(const Int val)
	{
		x = val;
		y = val;
		z = val;
	}

	inline void set(const Int _x, const Int _y, const Int _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	inline void set(const Int3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	inline void add(const Int3 &u)
	{
		x += u.x;
		y += u.y;
		z += u.z;
	}

	inline void sub(const Int3 &u)
	{
		x -= u.x;
		y -= u.y;
		z -= u.z;
	}

	inline void scale(const Real val)
	{
		x = Int(x*val);
		y = Int(y*val);
		z = Int(z*val);
	}

	inline void lerp(const Real t, const Int3 & u, const Int3 & v)
	{
		x = Int(u.x + t * (v.x - u.x));
		y = Int(u.y + t * (v.y - u.y));
		z = Int(u.z + t * (v.z - u.z));
	}

	inline Int3& operator=(const Int3 & u)
	{
		x = u.x; y = u.y; z = u.z;
        return *this;
	}

    inline bool operator==(const Int3 & u) const
    {
        return (u.x == x && u.y == y && u.z == z);
    }

    inline bool operator!=( const Int3& rhs ) const
    {
        return !(*this == rhs );
    }

    inline Int3 & operator*=(const Real lambda)
    {
        x = Int(x*lambda);
        y = Int(y*lambda);
        z = Int(z*lambda);
        return *this;
    }

	inline Int3 & operator/=(const Real lambda)
    {
		const Real invLambda = 1/lambda;
        x = Int(x*invLambda);
        y = Int(y*invLambda);
        z = Int(z*invLambda);
        return *this;
    }

	inline Int3 operator - () const
	{
		return Int3(-x, -y, -z);
	}

    inline Int3 & operator-=(const Int3 & u)
    {
        x -= u.x;
        y -= u.y;
        z -= u.z;
        return *this;
    }

    inline Int3 & operator+=(const Int3 & u)
    {
        x += u.x;
        y += u.y;
        z += u.z;
        return *this;
    }

    inline Int & operator[](int i)
    {
        return vec_array[i];
    }

    inline const Int operator[](int i) const
    {
        return vec_array[i];
    }

	inline operator const Int *() const { return vec_array; }

	inline friend Int dot(const Int3 & u, const Int3 & v)
	{
		return u.x*v.x + u.y*v.y + u.z*v.z;
	}

	inline friend Int3 & madd(Int3 & u, const Int3& v, const Real lambda)
	{
	    u.x += Int(v.x*lambda);
	    u.y += Int(v.y*lambda);
	    u.z += Int(v.z*lambda);
		return u;
	}

	inline friend Int3 & mult(Int3 & u, const Int3& v, const Real lambda)
	{
	    u.x = Int(v.x*lambda);
	    u.y = Int(v.y*lambda);
	    u.z = Int(v.z*lambda);
		return u;
	}

	inline friend Int3 & mult(Int3 & u, const Real lambda, const Int3& v)
	{
	    u.x = Int(v.x*lambda);
	    u.y = Int(v.y*lambda);
	    u.z = Int(v.z*lambda);
		return u;
	}

	inline friend Int3 & mult(Int3 & u, const Int3& v, const Int3& w)
	{
	    u.x = v.x*w.x;
	    u.y = v.y*w.y;
	    u.z = v.z*w.z;
		return u;
	}

	inline friend Int3 & sub(Int3 & u, const Int3& v, const Int3& w)
	{
	    u.x = v.x - w.x;
	    u.y = v.y - w.y;
	    u.z = v.z - w.z;
		return u;
	}

	inline friend Int3 & add(Int3 & u, const Int3& v, const Int3& w)
	{
	    u.x = v.x + w.x;
	    u.y = v.y + w.y;
	    u.z = v.z + w.z;
		return u;
	}

	inline friend Int3 & scale(Int3& u, const Real s)
	{
		u.x = Int(u.x*s);
		u.y = Int(u.y*s);
		u.z = Int(u.z*s);
		return u;
	}

	inline friend Int3 & scale(const Real s, Int3& u)
	{
		u.x = Int(u.x*s);
		u.y = Int(u.y*s);
		u.z = Int(u.z*s);
		return u;
	}

	inline friend Int3 & lerp(Int3 & w, Real t, const Int3 & u, const Int3 & v)
	{
		w.lerp(t, u, v);
		return w;
	}

	inline friend const Int3 operator+(const Int3& u, const Int3& v)
	{
		return Int3(u.x + v.x, u.y + v.y, u.z + v.z);
	}

	inline friend const Int3 operator-(const Int3& u, const Int3& v)
	{
	    return Int3(u.x - v.x, u.y - v.y, u.z - v.z);
	}

	inline friend const Int3 operator^(const Int3& u, const Int3& v)
	{
	    return Int3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
	}

	inline friend const Int3 operator*(const Real s, const Int3& u)
	{
		return Int3(Int(s*u.x), Int(s*u.y), Int(s*u.z));
	}

	inline friend const Int3 operator*(const Int3& u, const Real s)
	{
		return Int3(Int(s*u.x), Int(s*u.y), Int(s*u.z));
	}

	inline friend const Int3 operator/(const Int3& u, const Real s)
	{
		const Real invS = 1/s;
		return Int3(Int(u.x*invS), Int(u.y*invS), Int(u.z*invS));
	}

	inline friend Int operator*(const Int3& u, const Int3& v)
	{
		return u.x * v.x + u.y * v.y + u.z * v.z;
	}

	inline friend std::ostream& operator<<(std::ostream &o, const Int3& u)
	{
		return o << "(" << (int)u.x << "," << (int)u.y << "," << (int)u.z << ")";
	}

};

template <typename Int>
struct Int4
{
    union {
		struct {
			Int x,y,z,w;        // standard names for components
		};
		struct {
			Int r,g,b,a;
		};
        Int vec_array[4];     // array access
    };

	Int4() { }
	Int4(const Int v) : x(v), y(v), z(v), w(v) {}
    Int4(Int _x, Int _y, Int _z, Int _w)
		: x(_x), y(_y), z(_z), w(_w) { }
    Int4(const Int* xyzw) : x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3]) { }
	Int4(const Int4& u) : x(u.x), y(u.y), z(u.z), w(u.w) { }
	Int4(const Int4& _from, const Int4& _to)
		: x(_to.x - _from.x), y(_to.y - _from.y), z(_to.z - _from.z), w(_to.w - _from.w) {}

	inline void set(const Int val)
	{
		x = val;
		y = val;
		z = val;
		w = val;
	}

	inline void set(const Int _x, const Int _y, const Int _z, const Int _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	inline void set(const Int4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	inline void add(const Int4 &u)
	{
		x += u.x;
		y += u.y;
		z += u.z;
		w += u.w;
	}

	inline void sub(const Int4 &u)
	{
		x -= u.x;
		y -= u.y;
		z -= u.z;
		w -= u.w;
	}

	inline void scale(const Real val)
	{
		x = Int(x*val);
		y = Int(y*val);
		z = Int(z*val);
		w = Int(w*val);
	}

	inline void lerp(const Real t, const Int4 & u, const Int4 & v)
	{
		x = Int(u.x + t * (v.x - u.x));
		y = Int(u.y + t * (v.y - u.y));
		z = Int(u.z + t * (v.z - u.z));
		w = Int(u.w + t * (v.w - u.w));
	}

	inline Int4& operator=(const Int4 & u)
	{
		x = u.x; y = u.y; z = u.z; w = u.w;
        return *this;
	}

    inline bool operator==(const Int4 & u) const
    {
        return (u.x == x && u.y == y && u.z == z && u.w == w);
    }

    inline bool operator!=( const Int4& rhs ) const
    {
        return !(*this == rhs );
    }

    inline Int4 & operator*=(const Real lambda)
    {
		x = Int(x*lambda);
		y = Int(y*lambda);
		z = Int(z*lambda);
		w = Int(w*lambda);
		return *this;
    }

	inline Int4 & operator/=(const Real lambda)
    {
		const Real invLambda = 1/lambda;
		x = Int(x*invLambda);
		y = Int(y*invLambda);
		z = Int(z*invLambda);
		w = Int(w*invLambda);
		return *this;
    }

	inline Int4 operator - () const
	{
		return Int4(-x, -y, -z, -w);
	}

    inline Int4 & operator-=(const Int4 & u)
    {
		x -= u.x;
		y -= u.y;
		z -= u.z;
		w -= u.w;
		return *this;
    }

    inline Int4 & operator+=(const Int4 & u)
    {
		x += u.x;
		y += u.y;
		z += u.z;
		w += u.w;
		return *this;
    }

    inline Int & operator[](int i)
    {
        return vec_array[i];
    }

    inline const Int operator[](int i) const
    {
        return vec_array[i];
    }

	inline operator const Int *() const { return vec_array; }

	inline friend Int4 & madd(Int4 & u, const Int4& v, const Real lambda)
	{
		u.x += Int(v.x*lambda);
		u.y += Int(v.y*lambda);
		u.z += Int(v.z*lambda);
		u.w += Int(v.w*lambda);
		return u;
	}

	inline friend Int4 & mult(Int4 & u, const Int4& v, const Real lambda)
	{
		u.x = Int(v.x*lambda);
		u.y = Int(v.y*lambda);
		u.z = Int(v.z*lambda);
		u.w = Int(v.w*lambda);
		return u;
	}

	inline friend Int4 & mult(Int4 & u, const Real lambda, const Int4& v)
	{
	    u.x = Int(v.x*lambda);
	    u.y = Int(v.y*lambda);
	    u.z = Int(v.z*lambda);
	    u.w = Int(v.w*lambda);
		return u;
	}

	inline friend Int4 & mult(Int4 & u, const Int4& v, const Int4& w)
	{
	    u.x = v.x*w.x;
	    u.y = v.y*w.y;
	    u.z = v.z*w.z;
	    u.w = v.w*w.w;
		return u;
	}

	inline friend Int4 & sub(Int4 & u, const Int4& v, const Int4& w)
	{
		u.x = v.x - w.x;
		u.y = v.y - w.y;
		u.z = v.z - w.z;
		u.w = v.w - w.w;
		return u;
	}

	inline friend Int4 & add(Int4 & u, const Int4& v, const Int4& w)
	{
	    u.x = v.x + w.x;
	    u.y = v.y + w.y;
	    u.z = v.z + w.z;
	    u.w = v.w + w.w;
		return u;
	}

	inline friend Int4 & scale(Int4& u, const Real s)
	{
		u.x = Int(u.x*s);
		u.y = Int(u.y*s);
		u.z = Int(u.z*s);
		u.w = Int(u.w*s);
		return u;
	}

	inline friend Int4 & scale(const Real s, Int4& u)
	{
		u.x = Int(u.x*s);
		u.y = Int(u.y*s);
		u.z = Int(u.z*s);
		u.w = Int(u.w*s);
		return u;
	}

	inline friend Int4 & lerp(Int4 & w, const Real t, const Int4 & u, const Int4 & v)
	{
		w.lerp(t, u, v);
		return w;
	}

	inline friend const Int4 operator+(const Int4& u, const Int4& v)
	{
		return Int4(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
	}

	inline friend const Int4 operator-(const Int4& u, const Int4& v)
	{
	    return Int4(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
	}

	inline friend const Int4 operator*(const Real s, const Int4& u)
	{
		return Int4(Int(s*u.x), Int(s*u.y), Int(s*u.z), Int(s*u.w));
	}

	inline friend const Int4 operator/(const Int4& u, const Real s)
	{
		const Real invS = 1/s;
		return Int4(Int(u.x*invS), Int(u.y*invS), Int(u.z*invS), Int(u.w*invS));
	}

	inline friend Int operator*(const Int4& u, const Int4& v)
	{
		return (u.x * v.x + u.y * v.y + u.z * v.z) / (u.w * v.w);
	}

	inline friend std::ostream& operator<<(std::ostream &o, const Int4& u)
	{
		return o << "(" << (int)u.x << "," << (int)u.y << "," << (int)u.z << "," << (int)u.w << ")";
	}

};

typedef unsigned char ubyte;

typedef Int2<ubyte> ubvec2;
typedef Int3<ubyte> ubvec3;
typedef Int4<ubyte> ubvec4;

typedef unsigned int uint;

typedef Int2<uint> uivec2;
typedef Int3<uint> uivec3;
typedef Int4<uint> uivec4;

typedef Int2<int> ivec2;
typedef Int3<int> ivec3;
typedef Int4<int> ivec4;

}

