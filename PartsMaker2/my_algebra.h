#ifndef _MY_ALGEBRA_H_
#define _MY_ALGEBRA_H_

#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

//using namespace std;

namespace MyAlgebra {

typedef float Real;

#define my_eps 10e-6f

#ifndef M_PI
#define M_PI 3.141592653
#endif

struct vec2;
struct vec3;
struct vec4;
struct mat2;
struct mat3;
struct mat4;
struct quat;	// quaternion

template<typename T> inline 
T sqr(const T& t) { return t * t; } 

// linear interpolation
inline Real lerp(Real t, Real a, Real b)
{ return a * (1 - t) + t * b; }

struct vec2
{
    union {
		struct {
			Real x,y;        // standard names for components
		};
		struct {
			Real s,t;
		};
        Real vec_array[2];     // array access
    };

	vec2() { }
	vec2(const Real v) : x(v), y(v) { }
    vec2(const Real _x, const Real _y) : x(_x), y(_y) { }
    vec2(const Real* xy) : x(xy[0]), y(xy[1]) { }
	vec2(const vec2& u) : x(u.x), y(u.y) { }
	vec2(const vec2& u1, const vec2& u2)
		: x(u2.x - u1.x), y(u2.y - u1.y) {}

	inline void set(const Real val)
	{
		x = val;
		y = val;
	}

	inline void set(const Real _x, const Real _y)
	{
		x = _x;
		y = _y;
	}

	inline void set(const vec2 &v)
	{
		x = v.x;
		y = v.y;
	}

	inline void add(const vec2 &u)
	{
		x += u.x;
		y += u.y;
	}

	inline void sub(const vec2 &u)
	{
		x -= u.x;
		y -= u.y;
	}

	inline void scale(const Real val)
	{
		x *= val;
		y *= val;
	}

	inline Real dot(const vec2 & u) const
	{
		return (x * u.x + y * u.y);
	}

	// calc radian between 0 to M_PI
	inline Real angle(const vec2 &u) const
	{
//		return acos(dot(u));
		// return (double)Math.acos(dot(v1)/v1.length()/v.length());
		// Numerically, near 0 and PI are very bad condition for acos.
		// In 3-space, |atan2(sin,cos)| is much stable.
		
		const float c[] = { y-u.y, u.x-x, x*u.y-u.x*y };

		const Real sin = sqrtf(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
	
		return fabs(atan2(sin, dot(u)));
	}

	// calc radian between 0 to 2*M_PI
	// returns radian R, 
	// R is smaller than M_PI if ((*this) ^ u) * vec3(0,0,1) < 0,
	// and larger otherwise.
	inline Real angleFull(const vec2 &u) const
	{
		const float c[] = { y-u.y, u.x-x, x*u.y-u.x*y };

		const Real sin = sqrtf(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);

		//return (c[2] >= 0) ? fabs(atan2(sin, cos))
		//return (c[2] < 0) ? fabs(atan2(sin, dot(u)))
		//	: (Real)(2*M_PI - fabs(atan2(sin, dot(u))));
		return (c[2] < 0) ? (Real)acos(dot(u)) : (Real)(2.f*M_PI - acos(dot(u)));
	}

	// calc radian between -M_PI to M_PI
	// returns radian R, 
	// R is smaller than M_PI if ((*this) ^ u) * vec3(0,0,1) < 0,
	// and larger otherwise.
	inline Real angleNeg(const vec2 &u) const
	{
		const float c[] = { y-u.y, u.x-x, x*u.y-u.x*y };

		const Real sin = sqrtf(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);

		//return (c[2] >= 0) ? fabs(atan2(sin, cos))
		//return (c[2] < 0) ? fabs(atan2(sin, dot(u)))
		//	: (Real)(2*M_PI - fabs(atan2(sin, dot(u))));
		return (c[2] < 0) ? (Real)acos(dot(u)) : -(Real)acos(dot(u));
	}

	inline void rotate(Real angle)
	{
		const Real c = cosf(angle);
		const Real s = sinf(angle);

		const Real xNew = x * c - y * s;
		const Real yNew = x * s + y * c;

		x = xNew;
		y = yNew;
	}

	inline Real squareDistance(const vec2 &u) const
	{
		const vec2 v(*this, u);
		return v.dot(v);
	}

	inline Real distance(const vec2 &u) const
	{
		return sqrt(squareDistance(u));
	}

	inline void negate()
	{
		x = -x;
		y = -y;
	}

	inline void normalize()
	{
		const Real len = norm();
		x /= len;
		y /= len;
	}

	inline Real sq_norm() const { return x * x + y * y; }
	inline Real norm() const { return sqrtf(sq_norm()); }

	inline void lerp(const Real t, const vec2 & u, const vec2 & v)
	{
		x = MyAlgebra::lerp(t, u.x, v.x);
		y = MyAlgebra::lerp(t, u.y, v.y);
	}

	inline void orthogonalize( const vec2& v )
	{
		//  determine the orthogonal projection of this on v : dot( v , this ) * v
		//  and subtract it from this resulting in the orthogonalized this
		const vec2 res = dot(v) * v;
		x -= res.x;
		y -= res.y;
	}

	inline void orthonormalize( const vec2& v )
	{
		orthogonalize( v ); //  just orthogonalize...
		normalize();        //  ...and normalize it
	}

	inline vec2& operator=(const vec2 & u)
	{
		x = u.x;
		y = u.y;
        return *this;
	}

    inline bool operator==(const vec2 & u) const
    {
        return (u.x == x && u.y == y);
    }

    inline bool operator!=( const vec2& rhs ) const
    {
        return !(*this == rhs );
    }

    inline vec2 & operator*=(const Real lambda)
    {
        x *= lambda;
        y *= lambda;
        return *this;
    }

	inline vec2 & operator/=(const Real lambda)
    {
        x /= lambda;
        y /= lambda;
        return *this;
    }

    inline vec2 operator - () const
	{
		return vec2(-x, -y);
	}

	inline vec2 & operator-=(const vec2 & u)
    {
        x -= u.x;
        y -= u.y;
        return *this;
    }

    inline vec2 & operator+=(const vec2 & u)
    {
        x += u.x;
        y += u.y;
        return *this;
    }

    inline Real & operator[](const int i)
    {
        return vec_array[i];
    }

    inline const Real operator[](const int i) const
    {
        return vec_array[i];
    }

	inline operator const Real *() const { return vec_array; }

	inline friend Real dot(const vec2 & u, const vec2 & v)
	{
		return u.x*v.x + u.y*v.y;
	}

	inline friend vec2 & madd(vec2 & u, const vec2& v, const Real& lambda)
	{
	    u.x += v.x*lambda;
	    u.y += v.y*lambda;
		return u;
	}

	inline friend vec2 & mult(vec2 & u, const vec2& v, const Real& lambda)
	{
	    u.x = v.x*lambda;
	    u.y = v.y*lambda;
		return u;
	}

	inline friend vec2 & mult(vec2 & u, const Real& lambda, const vec2& v)
	{
	    u.x = v.x*lambda;
	    u.y = v.y*lambda;
		return u;
	}

	inline friend vec2 & mult(vec2 & u, const vec2& v, const vec2& w)
	{
	    u.x = v.x*w.x;
	    u.y = v.y*w.y;
		return u;
	}

	inline friend vec2 & sub(vec2 & u, const vec2& v, const vec2& w)
	{
	    u.x = v.x - w.x;
	    u.y = v.y - w.y;
		return u;
	}

	inline friend vec2 & add(vec2 & u, const vec2& v, const vec2& w)
	{
	    u.x = v.x + w.x;
	    u.y = v.y + w.y;
		return u;
	}

	inline friend vec2 & scale(vec2& u, const Real s)
	{
	    u.x *= s;
	    u.y *= s;
		return u;
	}

	inline friend vec2 & scale(const Real s, vec2& u)
	{
	    u.x *= s;
	    u.y *= s;
		return u;
	}

	inline friend vec2 & lerp(vec2 & w, Real t, const vec2 & u, const vec2 & v)
	{
		w.lerp(t, u, v);
		return w;
	}

	inline friend const vec2 normalize(const vec2& v)
	{
		const float l = v.norm();
		return vec2( v.x/l, v.y/l );
	}

	inline friend const vec2 operator+(const vec2& u, const vec2& v)
	{
		return vec2(u.x + v.x, u.y + v.y);
	}

	inline friend const vec2 operator-(const vec2& u, const vec2& v)
	{
	    return vec2(u.x - v.x, u.y - v.y);
	}

	inline friend const vec2 operator*(const Real s, const vec2& u)
	{
		return vec2(s * u.x, s * u.y);
	}

	inline friend const vec2 operator/(const vec2& u, const Real s)
	{
		return vec2(u.x / s, u.y / s);
	}

	inline friend Real operator*(const vec2& u, const vec2& v)
	{
		return u.x * v.x + u.y * v.y;
	}

	inline friend std::ostream& operator<<(std::ostream &o, const vec2& u)
	{
		return o << "(" << u.x << "," << u.y << ")";
	}

};


struct vec3
{
    union {
		struct {
			Real x,y,z;        // standard names for components
		};
		struct {
			Real r,g,b;
		};
        Real vec_array[3];     // array access
    };

	vec3() { }
	vec3(Real v) : x(v), y(v), z(v) { }
    vec3(const Real _x, const Real _y, const Real _z)
		: x(_x), y(_y), z(_z) { }
    vec3(const Real* xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]) { }
	vec3(const vec3& u) : x(u.x), y(u.y), z(u.z) { }
	vec3(const vec3& u1, const vec3& u2)
		: x(u2.x - u1.x), y(u2.y - u1.y), z(u2.z - u1.z) {}

	inline void set(const Real val)
	{
		x = val;
		y = val;
		z = val;
	}

	inline void set(const Real _x, const Real _y, const Real _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	inline void set(const vec3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	inline void add(const vec3 &u)
	{
		x += u.x;
		y += u.y;
		z += u.z;
	}

	inline void sub(const vec3 &u)
	{
		x -= u.x;
		y -= u.y;
		z -= u.z;
	}

	inline void scale(const Real val)
	{
		x *= val;
		y *= val;
		z *= val;
	}

	inline void cross(const vec3 & u, const vec3 & v)
	{
	    x = u.y * v.z - u.z * v.y;
		y = u.z * v.x - u.x * v.z;
		z = u.x * v.y - u.y * v.x;
	}

	inline Real dot(const vec3 & u) const
	{
		return (x * u.x + y * u.y + z * u.z);
	}

	// calc radian between 0 to M_PI
	Real angle(const vec3 &u) const
	{
//		return acos(dot(u));
		// return (double)Math.acos(dot(v1)/v1.length()/v.length());
		// Numerically, near 0 and PI are very bad condition for acos.
		// In 3-space, |atan2(sin,cos)| is much stable.
		//vec3 c;
		//c.cross(*this, u);
		const vec3 c = (*this) ^ u;
	
		const Real sin = c.norm();
	
		return fabs(atan2(sin, dot(u)));
	}

	// calc radian between 0 to 2*M_PI
	Real angleFull(const vec3 &u, const vec3 &normal) const
	{
		const vec3 c = (*this) ^ u;

		const Real sin = c.norm();

		return (c * normal >= 0) ? fabs(atan2(sin, dot(u)))
			: (Real)(2*M_PI - fabs(atan2(sin, dot(u))));
	}

	inline Real squareDistance(const vec3 &u) const
	{
		vec3 v(*this, u);
		return v.dot(v);
	}

	inline Real distance(const vec3 &u) const
	{
		return sqrt(squareDistance(u));
	}

	inline void negate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	inline void normalize()
	{
		const Real len = norm();
		x /= len;
		y /= len;
		z /= len;
	}

	inline Real sq_norm() const { return x * x + y * y + z * z; }
	inline Real norm() const { return sqrtf(sq_norm()); }

	inline void lerp(const Real t, const vec3 & u, const vec3 & v)
	{
		x = MyAlgebra::lerp(t, u.x, v.x);
		y = MyAlgebra::lerp(t, u.y, v.y);
		z = MyAlgebra::lerp(t, u.z, v.z);
	}

	void getTwoOrthogonals(vec3 &u, vec3 &v)
	{
		if (0.0f != x)
			u.set(-y, x, 0.0f);
		else
			u.set(0.0f, z, -y);

		v.cross(*this, u);
	}

	inline void orthogonalize( const vec3& v )
	{
		//  determine the orthogonal projection of this on v : dot( v , this ) * v
		//  and subtract it from this resulting in the orthogonalized this
		const vec3 res = dot(v) * v;
		x -= res.x;
		y -= res.y;
		z -= res.z;
	}

	inline void orthonormalize( const vec3& v )
	{
		orthogonalize( v ); //  just orthogonalize...
		normalize();        //  ...and normalize it
	}

	vec3 & lmult(const mat3& M);
	vec3 & rmult(const mat3& M);

	inline vec3& operator=(const vec3 & u)
	{
		x = u.x; y = u.y; z = u.z;
        return *this;
	}

    inline bool operator==(const vec3 & u) const
    {
        return (u.x == x && u.y == y && u.z == z);
    }

    inline bool operator!=( const vec3& rhs ) const
    {
        return !(*this == rhs );
    }

    inline vec3 & operator*=(const Real lambda)
    {
        x *= lambda;
        y *= lambda;
        z *= lambda;
        return *this;
    }

	inline vec3 & operator/=(const Real lambda)
    {
        x /= lambda;
        y /= lambda;
        z /= lambda;
        return *this;
    }

	inline vec3 operator - () const
	{
		return vec3(-x, -y, -z);
	}

    inline vec3 & operator-=(const vec3 & u)
    {
        x -= u.x;
        y -= u.y;
        z -= u.z;
        return *this;
    }

    inline vec3 & operator+=(const vec3 & u)
    {
        x += u.x;
        y += u.y;
        z += u.z;
        return *this;
    }

    inline Real & operator[](int i)
    {
        return vec_array[i];
    }

    inline const Real operator[](int i) const
    {
        return vec_array[i];
    }

	inline operator const Real *() const { return vec_array; }

	inline friend Real dot(const vec3 & u, const vec3 & v)
	{
		return u.x*v.x + u.y*v.y + u.z*v.z;
	}

	inline friend vec3 & madd(vec3 & u, const vec3& v, const Real& lambda)
	{
	    u.x += v.x*lambda;
	    u.y += v.y*lambda;
	    u.z += v.z*lambda;
		return u;
	}

	inline friend vec3 & mult(vec3 & u, const vec3& v, const Real& lambda)
	{
	    u.x = v.x*lambda;
	    u.y = v.y*lambda;
	    u.z = v.z*lambda;
		return u;
	}

	inline friend vec3 & mult(vec3 & u, const Real& lambda, const vec3& v)
	{
	    u.x = v.x*lambda;
	    u.y = v.y*lambda;
	    u.z = v.z*lambda;
		return u;
	}

	inline friend vec3 & mult(vec3 & u, const vec3& v, const vec3& w)
	{
	    u.x = v.x*w.x;
	    u.y = v.y*w.y;
	    u.z = v.z*w.z;
		return u;
	}

	inline friend vec3 & sub(vec3 & u, const vec3& v, const vec3& w)
	{
	    u.x = v.x - w.x;
	    u.y = v.y - w.y;
	    u.z = v.z - w.z;
		return u;
	}

	inline friend vec3 & add(vec3 & u, const vec3& v, const vec3& w)
	{
	    u.x = v.x + w.x;
	    u.y = v.y + w.y;
	    u.z = v.z + w.z;
		return u;
	}

	inline friend vec3 & scale(vec3& u, const Real s)
	{
	    u.x *= s;
	    u.y *= s;
	    u.z *= s;
		return u;
	}

	inline friend vec3 & scale(const Real s, vec3& u)
	{
	    u.x *= s;
	    u.y *= s;
	    u.z *= s;
		return u;
	}

	inline friend vec3 & lerp(vec3 & w, Real t, const vec3 & u, const vec3 & v)
	{
		w.lerp(t, u, v);
		return w;
	}

	inline friend vec3 normalize(const vec3& v)
	{
		const float l = v.norm();
		return vec3( v.x/l, v.y/l, v.z/l );
	}

	inline friend const vec3 operator+(const vec3& u, const vec3& v)
	{
		return vec3(u.x + v.x, u.y + v.y, u.z + v.z);
	}

	inline friend const vec3 operator-(const vec3& u, const vec3& v)
	{
	    return vec3(u.x - v.x, u.y - v.y, u.z - v.z);
	}

	inline friend const vec3 operator^(const vec3& u, const vec3& v)
	{
	    return vec3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
	}

	inline friend const vec3 operator*(const Real s, const vec3& u)
	{
		return vec3(s * u.x, s * u.y, s * u.z);
	}

	inline friend const vec3 operator*(const vec3& u, const Real s)
	{
		return vec3(s * u.x, s * u.y, s * u.z);
	}

	inline friend const vec3 operator/(const vec3& u, const Real s)
	{
		return vec3(u.x / s, u.y / s, u.z / s);
	}

	inline friend Real operator*(const vec3& u, const vec3& v)
	{
		return u.x * v.x + u.y * v.y + u.z * v.z;
	}

	inline friend std::ostream& operator<<(std::ostream &o, const vec3& u)
	{
		return o << "(" << u.x << "," << u.y << "," << u.z << ")";
	}

};


struct vec4
{
    union {
		struct {
			Real x,y,z,w;        // standard names for components
		};
		struct {
			Real r,g,b,a;
		};
        Real vec_array[4];     // array access
    };

	vec4() { }
	vec4(Real v) : x(v), y(v), z(v), w(v) { }
    vec4(Real _x, Real _y, Real _z, Real _w)
		: x(_x), y(_y), z(_z), w(_w) { }
    vec4(const Real* xyzw) : x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3]) { }
	vec4(const vec4& u) : x(u.x), y(u.y), z(u.z), w(u.w) { }
	vec4(const vec4& _from, const vec4& _to)
		: x(_to.x - _from.x), y(_to.y - _from.y), z(_to.z - _from.z), w(_to.w - _from.w) {}

	inline void set(const Real val)
	{
		x = val;
		y = val;
		z = val;
		w = val;
	}

	inline void set(const Real _x, const Real _y, const Real _z, const Real _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	inline void set(const vec4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	inline void add(const vec4 &u)
	{
		x += u.x;
		y += u.y;
		z += u.z;
		w += u.w;
	}

	inline void sub(const vec4 &u)
	{
		x -= u.x;
		y -= u.y;
		z -= u.z;
		w -= u.w;
	}

	inline void scale(const Real val)
	{
		x *= val;
		y *= val;
		z *= val;
		w *= val;
	}

	//inline void cross(const vec4 & u, const vec4 & v)
	//{
	//	x = u.y * v.z - u.z * v.y;
	//	y = u.z * v.x - u.x * v.z;
	//	z = u.x * v.y - u.y * v.x;
	//}

	inline Real dot(const vec4 & u) const
	{
		return (x * u.x + y * u.y + z * u.z) / (w * u.w);
	}

//	// calc radian between 0 to M_PI
//	Real angle(const vec4 &u) const
//	{
////		return acos(dot(u));
//		// return (double)Math.acos(dot(v1)/v1.length()/v.length());
//		// Numerically, near 0 and PI are very bad condition for acos.
//		// In 3-space, |atan2(sin,cos)| is much stable.
//		//vec4 c;
//		//c.cross(*this, u);
//		const vec4 c = (*this) ^ u;
//	
//		const Real sin = c.norm();
//	
//		return fabs(atan2(sin, dot(u)));
//	}

	//// calc radian between 0 to 2*M_PI
	//Real angleFull(const vec4 &u, const vec4 &normal) const
	//{
	//	const vec4 c = (*this) ^ u;

	//	const Real sin = c.norm();

	//	return (c * normal >= 0) ? fabs(atan2(sin, dot(u)))
	//		: (Real)(2*M_PI - fabs(atan2(sin, dot(u))));
	//}

	//inline Real squareDistance(const vec4 &u) const
	//{
	//	vec4 v(*this, u);
	//	return v.dot(v);
	//}

	//inline Real distance(const vec4 &u) const
	//{
	//	return sqrt(squareDistance(u));
	//}

	inline void negate()
	{
		x = -x;
		y = -y;
		z = -z;
		w = -w;
	}

	//inline void normalize()
	//{
	//	const Real len = norm();
	//	x /= len;
	//	y /= len;
	//	z /= len;
	//}

	//inline Real sq_norm() const { return x * x + y * y + z * z; }
	//inline Real norm() const { return sqrtf(sq_norm()); }

	inline void lerp(const Real t, const vec4 & u, const vec4 & v)
	{
		x = MyAlgebra::lerp(t, u.x, v.x);
		y = MyAlgebra::lerp(t, u.y, v.y);
		z = MyAlgebra::lerp(t, u.z, v.z);
		w = MyAlgebra::lerp(t, u.w, v.w);
	}

	//void getTwoOrthogonals(vec4 &u, vec4 &v)
	//{
	//	if (0.0f != x)
	//		u.set(-y, x, 0.0f);
	//	else
	//		u.set(0.0f, z, -y);

	//	v.cross(*this, u);
	//}

	//inline void orthogonalize( const vec4& v )
	//{
	//	//  determine the orthogonal projection of this on v : dot( v , this ) * v
	//	//  and subtract it from this resulting in the orthogonalized this
	//	const vec4 res = dot(v) * v;
	//	x -= res.x;
	//	y -= res.y;
	//	z -= res.z;
	//}

	//inline void orthonormalize( const vec4& v )
	//{
	//	orthogonalize( v ); //  just orthogonalize...
	//	normalize();        //  ...and normalize it
	//}

	vec4 & lmult(const mat4& M);
	vec4 & rmult(const mat4& M);

	inline vec4& operator=(const vec4 & u)
	{
		x = u.x; y = u.y; z = u.z; w = u.w;
        return *this;
	}

    inline bool operator==(const vec4 & u) const
    {
        return (u.x == x && u.y == y && u.z == z && u.w == w);
    }

    inline bool operator!=( const vec4& rhs ) const
    {
        return !(*this == rhs );
    }

    inline vec4 & operator*=(const Real lambda)
    {
		x *= lambda;
		y *= lambda;
		z *= lambda;
		w *= lambda;
		return *this;
    }

	inline vec4 & operator/=(const Real lambda)
    {
		x /= lambda;
		y /= lambda;
		z /= lambda;
		w /= lambda;
		return *this;
    }

	inline vec4 operator - () const
	{
		return vec4(-x, -y, -z, -w);
	}

    inline vec4 & operator-=(const vec4 & u)
    {
		x -= u.x;
		y -= u.y;
		z -= u.z;
		w -= u.w;
		return *this;
    }

    inline vec4 & operator+=(const vec4 & u)
    {
		x += u.x;
		y += u.y;
		z += u.z;
		w += u.w;
		return *this;
    }

    inline Real & operator[](int i)
    {
        return vec_array[i];
    }

    inline const Real operator[](int i) const
    {
        return vec_array[i];
    }

	inline operator const Real *() const { return vec_array; }

	inline friend Real dot(const vec4 & u, const vec4 & v)
	{
		return (u.x*v.x + u.y*v.y + u.z*v.z) / (u.w * v.w);
	}

	inline friend vec4 & madd(vec4 & u, const vec4& v, const Real& lambda)
	{
		u.x += v.x*lambda;
		u.y += v.y*lambda;
		u.z += v.z*lambda;
		u.w += v.w*lambda;
		return u;
	}

	inline friend vec4 & mult(vec4 & u, const vec4& v, const Real& lambda)
	{
		u.x = v.x*lambda;
		u.y = v.y*lambda;
		u.z = v.z*lambda;
		u.w = v.w*lambda;
		return u;
	}

	inline friend vec4 & mult(vec4 & u, const Real& lambda, const vec4& v)
	{
	    u.x = v.x*lambda;
	    u.y = v.y*lambda;
	    u.z = v.z*lambda;
	    u.w = v.w*lambda;
		return u;
	}

	inline friend vec4 & mult(vec4 & u, const vec4& v, const vec4& w)
	{
	    u.x = v.x*w.x;
	    u.y = v.y*w.y;
	    u.z = v.z*w.z;
	    u.w = v.w*w.w;
		return u;
	}

	inline friend vec4 & sub(vec4 & u, const vec4& v, const vec4& w)
	{
		u.x = v.x - w.x;
		u.y = v.y - w.y;
		u.z = v.z - w.z;
		u.w = v.w - w.w;
		return u;
	}

	inline friend vec4 & add(vec4 & u, const vec4& v, const vec4& w)
	{
	    u.x = v.x + w.x;
	    u.y = v.y + w.y;
	    u.z = v.z + w.z;
	    u.w = v.w + w.w;
		return u;
	}

	inline friend vec4 & scale(vec4& u, const Real s)
	{
		u.x *= s;
		u.y *= s;
		u.z *= s;
		u.w *= s;
		return u;
	}

	inline friend vec4 & scale(const Real s, vec4& u)
	{
	    u.x *= s;
	    u.y *= s;
	    u.z *= s;
	    u.w *= s;
		return u;
	}

	inline friend vec4 & lerp(vec4 & w, Real t, const vec4 & u, const vec4 & v)
	{
		w.lerp(t, u, v);
		return w;
	}

	inline friend const vec4 operator+(const vec4& u, const vec4& v)
	{
		return vec4(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
	}

	inline friend const vec4 operator-(const vec4& u, const vec4& v)
	{
	    return vec4(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
	}

	//inline friend const vec4 operator^(const vec4& u, const vec4& v)
	//{
	//    return vec4(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
	//}

	inline friend const vec4 operator*(const Real s, const vec4& u)
	{
		return vec4(s * u.x, s * u.y, s * u.z, s * u.w);
	}

	inline friend const vec4 operator/(const vec4& u, const Real s)
	{
		return vec4(u.x / s, u.y / s, u.z / s, u.w / s);
	}

	inline friend Real operator*(const vec4& u, const vec4& v)
	{
		return (u.x * v.x + u.y * v.y + u.z * v.z) / (u.w * v.w);
	}

	inline friend std::ostream& operator<<(std::ostream &o, const vec4& u)
	{
		return o << "(" << u.x << "," << u.y << "," << u.z << "," << u.w << ")";
	}

};



/*
    calculate the determinent of a 2x2 matrix in the from

    | a1 a2 |
    | b1 b2 |

*/
inline Real det2x2(Real a1, Real a2, Real b1, Real b2)
{
    return a1 * b2 - b1 * a2;
}

/*
    calculate the determinent of a 3x3 matrix in the from

    | a1 a2 a3 |
    | b1 b2 b3 |
    | c1 c2 c3 |

*/
inline Real det3x3(Real a1, Real a2, Real a3, 
                   Real b1, Real b2, Real b3, 
                   Real c1, Real c2, Real c3)
{
    return a1 * det2x2(b2, b3, c2, c3) - b1 * det2x2(a2, a3, c2, c3) + c1 * det2x2(a2, a3, b2, b3);
}


/*
    for all the matrices...a<x><y> indicates the element at row x, col y

    For example:
    a01 <-> row 0, col 1 
*/

struct mat2
{
	union {
		struct {
			Real a00, a10;
			Real a01, a11;
		};
		Real mat_array[4];
	};

	mat2() {}

	mat2(const Real* array)
	{
		memcpy(mat_array, array,sizeof(Real) * 4);
	}

	mat2(const mat2 & M)
	{
		memcpy(mat_array, M.mat_array, sizeof(Real) * 4);
	}

	mat2( const Real& f0, const Real& f1,
		  const Real& f2, const Real& f3 )
		  : a00( f0 ), a10( f1 ),
		    a01( f2 ), a11( f3 ) {}

    const vec2 col(const int i) const
    {
        return vec2(&mat_array[i * 2]);
    }
	
	void set_row(const int i, const vec2 & v)
    {
        mat_array[i] = v.x;
        mat_array[i + 2] = v.y;
    }

	void set_col(const int i, const vec2 & v)
	{
        mat_array[i * 2] = v.x;
        mat_array[i * 2 + 1] = v.y;
	}

	// theta in radians
	void set_rot(const Real& theta) 
	{
		const Real _cos = (Real)cos(theta);
		const Real _sin = (Real)sin(theta);

		a00 = _cos;
		a01 = -_sin;
		a10 = _sin;
		a11 = _cos;
	}

	void transpose()
	{
	    Real tmp;
	    tmp = a01;
	    a01 = a10;
	    a10 = tmp;
	}
	
	void negate()
	{
		a00 = -a00;	a01 = -a01;
		a10 = -a10; a11 = -a11;
	}

	inline const Real det() const
	{
	    return a00 * a11 - a01 * a10;
	}

	void invert()
	{
		const Real oodet = 1.f / det();

		const Real tmp = a00;
		a00 = a11;
		a11 = tmp;

		a01 = -a01;
		a10 = -a10;

		a00 *= oodet; a01 *= oodet;
		a10 *= oodet; a11 *= oodet;
	}

	inline const Real trace() const
	{
		return a00 + a11;
	}

    const vec2 operator[](const int i) const
    {
        return vec2(mat_array[i], mat_array[i + 2]);
    }

    inline const Real& operator()(const int& i, const int& j) const
    {
        return mat_array[ j * 2 + i ];
    }

    inline Real& operator()(const int& i, const int& j)
    {
        return  mat_array[ j * 2 + i ];
    }

	inline Real operator[](const int i) { return mat_array[i]; }

	inline operator const Real *() const { return mat_array; }

    inline mat2 & operator*=(const Real & lambda)
    {
		a00 *= lambda; a01 *= lambda;
		a10 *= lambda; a11 *= lambda;
        return *this;
    }

    inline mat2 & operator-=(const mat2 & M)
    {
		a00 -= M.a00; a01 -= M.a01;
		a10 -= M.a10; a11 -= M.a11;
        return *this;
    }

    inline mat2 & operator+=(const mat2 & M)
    {
		a00 += M.a00; a01 += M.a01;
		a10 += M.a10; a11 += M.a11;
        return *this;
    }

	friend const vec2 operator*(const mat2& M, const vec2& v)
	{
		vec2 u;
	    u.x = M.a00 * v.x + M.a01 * v.y;
	    u.y = M.a10 * v.x + M.a11 * v.y;
	    return u;
	}

	friend const vec2 operator*(const vec2& v, const mat2& M)
	{
		vec2 u;
	    u.x = v.x * M.a00 + v.y * M.a10;
	    u.y = v.x * M.a01 + v.y * M.a11;
	    return u;
	}

	friend vec2 & mult(vec2& u, const mat2& M, const vec2& v)
	{
	    u.x = M.a00 * v.x + M.a01 * v.y;
	    u.y = M.a10 * v.x + M.a11 * v.y;
	    return u;
	}

	friend vec2 & mult(vec2& u, const vec2& v, const mat2& M)
	{
	    u.x = M.a00 * v.x + M.a10 * v.y;
	    u.y = M.a01 * v.x + M.a11 * v.y;
	    return u;
	}

	friend mat2 & add(mat2& A, const mat2& B)
	{
	    A.a00 += B.a00;
	    A.a10 += B.a10;
	    A.a01 += B.a01;
	    A.a11 += B.a11;
	    return A;
	}

	friend mat2 & add(mat2 & C, const mat2 & A, const mat2 & B)
	{
		// If there is selfassignment involved
		// we can't go without a temporary.
		if (&C == &A || &C == &B)
		{
			mat2 mTemp;

	        mTemp.a00 = A.a00 + B.a00;
	        mTemp.a01 = A.a01 + B.a01;
	        mTemp.a10 = A.a10 + B.a10;
	        mTemp.a11 = A.a11 + B.a11;
   
	        C = mTemp;
	    }
	    else
	    {
	        C.a00 = A.a00 + B.a00;
	        C.a01 = A.a01 + B.a01;
	        C.a10 = A.a10 + B.a10;
	        C.a11 = A.a11 + B.a11;
	    }
	    return C;
	}

	// C = A * B

	// C.a00 C.a01   A.a00 A.a01   B.a00 B.a01
	//                                                          
	// C.a10 C.a11   A.a10 A.a11   B.a10 B.a11

	friend mat2 & mult(mat2& C, const mat2& A, const mat2& B)
	{
                                // If there is self-assignment involved
                                // we can't go without a temporary.
	    if (&C == &A || &C == &B)
	    {
	        mat2 mTemp;
	
	        mTemp.a00 = A.a00 * B.a00 + A.a01 * B.a10;
	        mTemp.a10 = A.a10 * B.a00 + A.a11 * B.a10;
	        mTemp.a01 = A.a00 * B.a01 + A.a01 * B.a11;
	        mTemp.a11 = A.a10 * B.a01 + A.a11 * B.a11;
	
	        C = mTemp;
	    }
	    else
	    {
	        C.a00 = A.a00 * B.a00 + A.a01 * B.a10;
	        C.a10 = A.a10 * B.a00 + A.a11 * B.a10;
	        C.a01 = A.a00 * B.a01 + A.a01 * B.a11;
	        C.a11 = A.a10 * B.a01 + A.a11 * B.a11;
	    }

	    return C;
	}

	friend mat2 & transpose(mat2& A)
	{
	    Real tmp;
	    tmp = A.a01;
	    A.a01 = A.a10;
	    A.a10 = tmp;
	    return A;
	}

	friend mat2 & transpose(mat2& B, const mat2& A)
	{
	    B.a00 = A.a00;
	    B.a01 = A.a10;
	    B.a10 = A.a01;
	    B.a11 = A.a11;
	    return B;
	}

	inline friend Real det(const mat2& A)
	{
	    return A.det();
	}

	friend mat2 & invert(mat2& B, const mat2& A)
	{
		const Real oodet = 1.f / A.det();

		B.a00 =  A.a11 * oodet;
		B.a01 = -A.a01 * oodet;
		B.a10 = -A.a10 * oodet;
		B.a11 =  A.a00 * oodet;

		return B;
	}

	friend mat2 & negate(mat2 & M)
	{
		M.a00 = -M.a00;
		M.a01 = -M.a01;
		M.a10 = -M.a10;
		M.a11 = -M.a11;
	    return M;
	}
};

struct mat3
{
    union {
        struct {
            Real a00, a10, a20;        // standard names for components
            Real a01, a11, a21;        // standard names for components
            Real a02, a12, a22;        // standard names for components
        };
        Real mat_array[9];     // array access
    };

	mat3() {};
    
	mat3(const Real* array)
	{
    	memcpy(mat_array, array, sizeof(Real) * 9);
	}
	
	mat3(const mat3 & M)
	{
	    memcpy(mat_array, M.mat_array, sizeof(Real) * 9);
	}
	
    mat3( const Real& f0,  const Real& f1,  const Real& f2,  
          const Real& f3,  const Real& f4,  const Real& f5,  
          const Real& f6,  const Real& f7,  const Real& f8 )
  		  : a00( f0 ), a10( f1 ), a20( f2 ), 
            a01( f3 ), a11( f4 ), a21( f5 ),
  		    a02( f6 ), a12( f7 ), a22( f8) { }

    const vec3 col(const int i) const
    {
        return vec3(&mat_array[i * 3]);
    }
	
	void set_row(const int i, const vec3 & v)
    {
        mat_array[i] = v.x;
        mat_array[i + 3] = v.y;
        mat_array[i + 6] = v.z;
    }

	void set_col(const int i, const vec3 & v)
	{
        mat_array[i * 3] = v.x;
        mat_array[i * 3 + 1] = v.y;
        mat_array[i * 3 + 2] = v.z;
	}

	// v is normalized
	// theta in radians
	void set_rot(const Real& theta, const vec3& v) 
	{
	    const Real ct = Real(cos(theta));
	    const Real st = Real(sin(theta));

	    const Real xx = v.x * v.x;
	    const Real yy = v.y * v.y;
	    const Real zz = v.z * v.z;
	    const Real xy = v.x * v.y;
	    const Real xz = v.x * v.z;
	    const Real yz = v.y * v.z;
	
	    a00 = xx + ct*(1-xx);
	    a01 = xy + ct*(-xy) + st*-v.z;
	    a02 = xz + ct*(-xz) + st*v.y;

	    a10 = xy + ct*(-xy) + st*v.z;
	    a11 = yy + ct*(1-yy);
	    a12 = yz + ct*(-yz) + st*-v.x;

	    a20 = xz + ct*(-xz) + st*-v.y;
	    a21 = yz + ct*(-yz) + st*v.x;
	    a22 = zz + ct*(1-zz);
	}

	void set_rot(const vec3& u, const vec3& v)
	{
	    const vec3 w = u ^ v;
		const Real phi = u.dot(v);
		const Real lambda = w.dot(w);

	    const Real h = (lambda > my_eps) ? (1.0f - phi) / lambda : lambda;

//	    cross(w,u,v);
//	    phi = dot(u,v);
//	    lambda = dot(w,w);
	    //if (lambda > my_eps)
	    //    h = (1.0f - phi) / lambda;
	    //else
	    //    h = lambda;
    
	    const Real hxy = w.x * w.y * h;
	    const Real hxz = w.x * w.z * h;
	    const Real hyz = w.y * w.z * h;

	    a00 = phi + w.x * w.x * h;
	    a01 = hxy - w.z;
	    a02 = hxz + w.y;

	    a10 = hxy + w.z;
	    a11 = phi + w.y * w.y * h;
	    a12 = hyz - w.x;

	    a20 = hxz - w.y;
	    a21 = hyz + w.x;
	    a22 = phi + w.z * w.z * h;
	}

    // Matrix norms...
    // Compute || M ||
    //                1
//	Real mat3::norm_one()
	Real norm_one() const
	{
	    Real sum = fabs(a00) + fabs(a10) + fabs(a20);
		Real max = sum;

		sum = fabs(a01) + fabs(a11) + fabs(a21);
	    if (max < sum) max = sum;

		sum = fabs(a02) + fabs(a12) + fabs(a22);
	    if (max < sum) max = sum;

		return max;
	}

    // Compute || M ||
    //                +inf
	Real norm_inf() const
	{
	    Real sum = fabs(a00) + fabs(a01) + fabs(a02);
		Real max = sum;

		sum = fabs(a10) + fabs(a11) + fabs(a12);
	    if (max < sum) max = sum;

		sum = fabs(a20) + fabs(a21) + fabs(a22);
	    if (max < sum) max = sum;

		return max;
	}

	void transpose()
	{
	    Real tmp;
	    tmp = a01;
	    a01 = a10;
	    a10 = tmp;
	
	    tmp = a02;
	    a02 = a20;
	    a20 = tmp;
	
	    tmp = a12;
	    a12 = a21;
	    a21 = tmp;
	}
	
	void adjoint_transpose()
	{
		const mat3 A = *this;

		set_col(0, A.col(1) ^ A.col(2));
		set_col(1, A.col(2) ^ A.col(0));
		set_col(2, A.col(0) ^ A.col(1));
	}

	void negate()
	{
		a00 = -a00; a01 = -a01; a02 = -a02;
		a10 = -a10; a11 = -a11; a12 = -a12;
		a20 = -a20; a21 = -a21; a22 = -a22;
	}

	void invert()
	{
		mat3 B;
	
	    B.a00 =  (a11 * a22 - a21 * a12);
	    B.a10 = -(a10 * a22 - a20 * a12);
	    B.a20 =  (a10 * a21 - a20 * a11);
	    B.a01 = -(a01 * a22 - a21 * a02);
	    B.a11 =  (a00 * a22 - a20 * a02);
	    B.a21 = -(a00 * a21 - a20 * a01);
	    B.a02 =  (a01 * a12 - a11 * a02);
	    B.a12 = -(a00 * a12 - a10 * a02);
	    B.a22 =  (a00 * a11 - a10 * a01);
	
	    const Real det = (a00 * B.a00) + (a01 * B.a10) + (a02 * B.a20);
	    
	    const Real oodet = 1.0f / det;
	
	    B.a00 *= oodet; B.a01 *= oodet; B.a02 *= oodet;
	    B.a10 *= oodet; B.a11 *= oodet; B.a12 *= oodet;
	    B.a20 *= oodet; B.a21 *= oodet; B.a22 *= oodet;

		*this = B;
	}

	Real det() const
	{
	    return det3x3(a00, a01, a02, 
	                 a10, a11, a12, 
	                 a20, a21, a22);
	}

	inline Real trace() const
	{
		return a00 + a11 + a22;
	}

    inline vec3 operator[](const int i) const
    {
        return vec3(mat_array[i], mat_array[i + 3], mat_array[i + 6]);
    }

    inline Real& operator()(const int& i, const int& j)
    {
        return mat_array[ j * 3 + i ];
    }

    inline const Real operator()(const int& i, const int& j) const
    {
        return  mat_array[ j * 3 + i ];
    }

	inline Real& operator[](const int i) { return mat_array[i]; }

	inline operator const Real *() const { return mat_array; }

    inline mat3 & operator*=(const Real & lambda)
    {
		a00 *= lambda; a01 *= lambda; a02 *= lambda;
		a10 *= lambda; a11 *= lambda; a12 *= lambda;
		a20 *= lambda; a21 *= lambda; a22 *= lambda;

        return *this;
    }

    inline mat3 & operator-=(const mat3 & M)
    {
		a00 -= M.a00; a01 -= M.a01; a02 -= M.a02;
		a10 -= M.a10; a11 -= M.a11; a12 -= M.a12;
		a20 -= M.a20; a21 -= M.a21; a22 -= M.a22;

        return *this;
    }

    inline mat3 & operator+=(const mat3 & M)
    {
		a00 += M.a00; a01 += M.a01; a02 += M.a02;
		a10 += M.a10; a11 += M.a11; a12 += M.a12;
		a20 += M.a20; a21 += M.a21; a22 += M.a22;

        return *this;
    }

	friend const vec3 operator*(const mat3& M, const vec3& v)
	{
		vec3 u;
	    u.x = M.a00 * v.x + M.a01 * v.y + M.a02 * v.z;
	    u.y = M.a10 * v.x + M.a11 * v.y + M.a12 * v.z;
	    u.z = M.a20 * v.x + M.a21 * v.y + M.a22 * v.z;
	    return u;
	}

	friend const vec3 operator*(const vec3& v, const mat3& M)
	{
		vec3 u;
	    u.x = v.x * M.a00 + v.y * M.a10 + v.z * M.a20;
	    u.y = v.x * M.a01 + v.y * M.a11 + v.z * M.a21;
	    u.z = v.x * M.a02 + v.y * M.a12 + v.z * M.a22;
	    return u;
	}

	friend vec3 & mult(vec3& u, const mat3& M, const vec3& v)
	{
	    u.x = M.a00 * v.x + M.a01 * v.y + M.a02 * v.z;
	    u.y = M.a10 * v.x + M.a11 * v.y + M.a12 * v.z;
	    u.z = M.a20 * v.x + M.a21 * v.y + M.a22 * v.z;
	    return u;
	}

	friend vec3 & mult(vec3& u, const vec3& v, const mat3& M)
	{
	    u.x = M.a00 * v.x + M.a10 * v.y + M.a20 * v.z;
	    u.y = M.a01 * v.x + M.a11 * v.y + M.a21 * v.z;
	    u.z = M.a02 * v.x + M.a12 * v.y + M.a22 * v.z;
	    return u;
	}

	friend mat3 & add(mat3& A, const mat3& B)
	{
	    A.a00 += B.a00;
	    A.a10 += B.a10;
	    A.a20 += B.a20;
	    A.a01 += B.a01;
	    A.a11 += B.a11;
	    A.a21 += B.a21;
	    A.a02 += B.a02;
	    A.a12 += B.a12;
	    A.a22 += B.a22;
	    return A;
	}

	friend mat3 & add(mat3 & C, const mat3 & A, const mat3 & B)
	{
		// If there is selfassignment involved
		// we can't go without a temporary.
		if (&C == &A || &C == &B)
		{
			mat3 mTemp;

	        mTemp.a00 = A.a00 + B.a00;
	        mTemp.a01 = A.a01 + B.a01;
	        mTemp.a02 = A.a02 + B.a02;
	        mTemp.a10 = A.a10 + B.a10;
	        mTemp.a11 = A.a11 + B.a11;
	        mTemp.a12 = A.a12 + B.a12;
	        mTemp.a20 = A.a20 + B.a20;
	        mTemp.a21 = A.a21 + B.a21;
	        mTemp.a22 = A.a22 + B.a22;
   
	        C = mTemp;
	    }
	    else
	    {
	        C.a00 = A.a00 + B.a00;
	        C.a01 = A.a01 + B.a01;
	        C.a02 = A.a02 + B.a02;
	        C.a10 = A.a10 + B.a10;
	        C.a11 = A.a11 + B.a11;
	        C.a12 = A.a12 + B.a12;
	        C.a20 = A.a20 + B.a20;
	        C.a21 = A.a21 + B.a21;
	        C.a22 = A.a22 + B.a22;
	    }
	    return C;
	}

	// C = A * B

	// C.a00 C.a01 C.a02   A.a00 A.a01 A.a02   B.a00 B.a01 B.a02
	//                                                          
	// C.a10 C.a11 C.a12   A.a10 A.a11 A.a12   B.a10 B.a11 B.a12
	//                                                          
	// C.a20 C.a21 C.a22 = A.a20 A.a21 A.a22 * B.a20 B.a21 B.a22

	friend mat3 & mult(mat3& C, const mat3& A, const mat3& B)
	{
                                // If there is self-assignment involved
                                // we can't go without a temporary.
	    if (&C == &A || &C == &B)
	    {
	        mat3 mTemp;
	
	        mTemp.a00 = A.a00 * B.a00 + A.a01 * B.a10 + A.a02 * B.a20;
	        mTemp.a10 = A.a10 * B.a00 + A.a11 * B.a10 + A.a12 * B.a20;
	        mTemp.a20 = A.a20 * B.a00 + A.a21 * B.a10 + A.a22 * B.a20;
	        mTemp.a01 = A.a00 * B.a01 + A.a01 * B.a11 + A.a02 * B.a21;
	        mTemp.a11 = A.a10 * B.a01 + A.a11 * B.a11 + A.a12 * B.a21;
	        mTemp.a21 = A.a20 * B.a01 + A.a21 * B.a11 + A.a22 * B.a21;
	        mTemp.a02 = A.a00 * B.a02 + A.a01 * B.a12 + A.a02 * B.a22;
	        mTemp.a12 = A.a10 * B.a02 + A.a11 * B.a12 + A.a12 * B.a22;
	        mTemp.a22 = A.a20 * B.a02 + A.a21 * B.a12 + A.a22 * B.a22;
	
	        C = mTemp;
	    }
	    else
	    {
	        C.a00 = A.a00 * B.a00 + A.a01 * B.a10 + A.a02 * B.a20;
	        C.a10 = A.a10 * B.a00 + A.a11 * B.a10 + A.a12 * B.a20;
	        C.a20 = A.a20 * B.a00 + A.a21 * B.a10 + A.a22 * B.a20;
	        C.a01 = A.a00 * B.a01 + A.a01 * B.a11 + A.a02 * B.a21;
	        C.a11 = A.a10 * B.a01 + A.a11 * B.a11 + A.a12 * B.a21;
	        C.a21 = A.a20 * B.a01 + A.a21 * B.a11 + A.a22 * B.a21;
	        C.a02 = A.a00 * B.a02 + A.a01 * B.a12 + A.a02 * B.a22;
	        C.a12 = A.a10 * B.a02 + A.a11 * B.a12 + A.a12 * B.a22;
	        C.a22 = A.a20 * B.a02 + A.a21 * B.a12 + A.a22 * B.a22;
	    }

	    return C;
	}

	friend mat3 & transpose(mat3& A)
	{
	    Real tmp;
	    tmp = A.a01;
	    A.a01 = A.a10;
	    A.a10 = tmp;
	
	    tmp = A.a02;
	    A.a02 = A.a20;
	    A.a20 = tmp;
	
	    tmp = A.a12;
	    A.a12 = A.a21;
	    A.a21 = tmp;
	    return A;
	}

	friend mat3 & transpose(mat3& B, const mat3& A)
	{
	    B.a00 = A.a00;
	    B.a01 = A.a10;
	    B.a02 = A.a20;
	    B.a10 = A.a01;
	    B.a11 = A.a11;
	    B.a12 = A.a21;
	    B.a20 = A.a02;
	    B.a21 = A.a12;
	    B.a22 = A.a22;
	    return B;
	}

	friend mat3 & adjoint_transpose(mat3& A)
	{
		const mat3 B = A;

		A.set_col(0, B.col(1) ^ B.col(2));
		A.set_col(1, B.col(2) ^ B.col(0));
		A.set_col(2, B.col(0) ^ B.col(1));

		return A;
	}

	friend mat3 & adjoint_transpose(mat3& B, const mat3& A)
	{
		B.set_col(0, A.col(1) ^ A.col(2));
		B.set_col(1, A.col(2) ^ A.col(0));
		B.set_col(2, A.col(0) ^ A.col(1));

		return B;
	}

	friend Real det(const mat3& A)
	{
	    return det3x3(A.a00, A.a01, A.a02, 
	                 A.a10, A.a11, A.a12, 
	                 A.a20, A.a21, A.a22);
	}

	friend Real trace(const mat3& A)
	{
		return A.trace();
	}

	friend mat3 & invert(mat3& B, const mat3& A)
	{
	    B.a00 =  (A.a11 * A.a22 - A.a21 * A.a12);
	    B.a10 = -(A.a10 * A.a22 - A.a20 * A.a12);
	    B.a20 =  (A.a10 * A.a21 - A.a20 * A.a11);
	    B.a01 = -(A.a01 * A.a22 - A.a21 * A.a02);
	    B.a11 =  (A.a00 * A.a22 - A.a20 * A.a02);
	    B.a21 = -(A.a00 * A.a21 - A.a20 * A.a01);
	    B.a02 =  (A.a01 * A.a12 - A.a11 * A.a02);
	    B.a12 = -(A.a00 * A.a12 - A.a10 * A.a02);
	    B.a22 =  (A.a00 * A.a11 - A.a10 * A.a01);

	    const Real det = (A.a00 * B.a00) + (A.a01 * B.a10) + (A.a02 * B.a20);
    
	    const Real oodet = 1.0f / det;

	    B.a00 *= oodet; B.a01 *= oodet; B.a02 *= oodet;
	    B.a10 *= oodet; B.a11 *= oodet; B.a12 *= oodet;
	    B.a20 *= oodet; B.a21 *= oodet; B.a22 *= oodet;
	    return B;
	}

	friend mat3 & negate(mat3 & M)
	{
		M.a00 = -M.a00; M.a01 = -M.a01; M.a02 = -M.a02;
		M.a10 = -M.a10; M.a11 = -M.a11; M.a12 = -M.a12;
		M.a20 = -M.a20; M.a21 = -M.a21; M.a22 = -M.a22;

	    return M;
	}

};

struct mat4
{
    union {
        struct {
            Real a00, a10, a20, a30;        // standard names for components
            Real a01, a11, a21, a31;        // standard names for components
            Real a02, a12, a22, a32;        // standard names for components
            Real a03, a13, a23, a33;        // standard names for components
        };
        Real mat_array[16];     // array access
    };

	mat4() {};
    
	mat4(const Real* array)
	{
    	memcpy(mat_array, array, sizeof(Real) * 16);
	}
	
	mat4(const mat4 & M)
	{
	    memcpy(mat_array, M.mat_array, sizeof(Real) * 16);
	}
	
    mat4( const Real& f0,  const Real& f1,  const Real& f2,  const Real& f3,
		  const Real& f4,  const Real& f5,  const Real& f6,  const Real& f7,
		  const Real& f8,  const Real& f9,  const Real& f10, const Real& f11,
		  const Real& f12, const Real& f13, const Real& f14, const Real& f15)
  		  : a00( f0 ),  a10( f1 ),  a20( f2 ),  a30( f3 ),
            a01( f4 ),  a11( f5 ),  a21( f6 ),  a31( f7 ),
  		    a02( f8 ),  a12( f9 ),  a22( f10 ), a32( f11 ),
  		    a03( f12 ), a13( f13 ), a23( f14 ), a33( f15 ) { }

    const vec4 col(const int i) const
    {
        return vec4(&mat_array[i * 4]);
    }
	
	void set_row(const int i, const vec4 & v)
    {
        mat_array[i] = v.x;
        mat_array[i + 4] = v.y;
        mat_array[i + 8] = v.z;
        mat_array[i + 12] = v.w;
    }

	void set_col(const int i, const vec4 & v)
	{
        mat_array[i * 4] = v.x;
        mat_array[i * 4 + 1] = v.y;
        mat_array[i * 4 + 2] = v.z;
        mat_array[i * 4 + 3] = v.w;
	}

	//// v is normalized
	//// theta in radians
	//void set_rot(const Real& theta, const vec3& v) 
	//{
	//    const Real ct = Real(cos(theta));
	//    const Real st = Real(sin(theta));

	//    const Real xx = v.x * v.x;
	//    const Real yy = v.y * v.y;
	//    const Real zz = v.z * v.z;
	//    const Real xy = v.x * v.y;
	//    const Real xz = v.x * v.z;
	//    const Real yz = v.y * v.z;
	//
	//    a00 = xx + ct*(1-xx);
	//    a01 = xy + ct*(-xy) + st*-v.z;
	//    a02 = xz + ct*(-xz) + st*v.y;

	//    a10 = xy + ct*(-xy) + st*v.z;
	//    a11 = yy + ct*(1-yy);
	//    a12 = yz + ct*(-yz) + st*-v.x;

	//    a20 = xz + ct*(-xz) + st*-v.y;
	//    a21 = yz + ct*(-yz) + st*v.x;
	//    a22 = zz + ct*(1-zz);
	//}

//	void set_rot(const vec3& u, const vec3& v)
//	{
//	    const vec3 w = u ^ v;
//		const Real phi = u.dot(v);
//		const Real lambda = w.dot(w);
//
//	    const Real h = (lambda > my_eps) ? (1.0f - phi) / lambda : lambda;
//
////	    cross(w,u,v);
////	    phi = dot(u,v);
////	    lambda = dot(w,w);
//	    //if (lambda > my_eps)
//	    //    h = (1.0f - phi) / lambda;
//	    //else
//	    //    h = lambda;
//    
//	    const Real hxy = w.x * w.y * h;
//	    const Real hxz = w.x * w.z * h;
//	    const Real hyz = w.y * w.z * h;
//
//	    a00 = phi + w.x * w.x * h;
//	    a01 = hxy - w.z;
//	    a02 = hxz + w.y;
//
//	    a10 = hxy + w.z;
//	    a11 = phi + w.y * w.y * h;
//	    a12 = hyz - w.x;
//
//	    a20 = hxz - w.y;
//	    a21 = hyz + w.x;
//	    a22 = phi + w.z * w.z * h;
//	}

    // Matrix norms...
    // Compute || M ||
    //                1
	Real norm_one() const
	{
	    Real sum = fabs(a00) + fabs(a10) + fabs(a20) + fabs(a30);
		Real max = sum;

		sum = fabs(a01) + fabs(a11) + fabs(a21) + fabs(a31);
	    if (max < sum) max = sum;

		sum = fabs(a02) + fabs(a12) + fabs(a22) + fabs(a32);
	    if (max < sum) max = sum;

		sum = fabs(a03) + fabs(a13) + fabs(a23) + fabs(a33);
	    if (max < sum) max = sum;

		return max;
	}

    // Compute || M ||
    //                +inf
	Real norm_inf() const
	{
	    Real sum = fabs(a00) + fabs(a01) + fabs(a02) + fabs(a03);
		Real max = sum;

		sum = fabs(a10) + fabs(a11) + fabs(a12) + fabs(a13);
	    if (max < sum) max = sum;

		sum = fabs(a20) + fabs(a21) + fabs(a22) + fabs(a23);
	    if (max < sum) max = sum;

		sum = fabs(a30) + fabs(a31) + fabs(a32) + fabs(a33);
		if (max < sum) max = sum;
		
		return max;
	}

	void transpose()
	{
	    Real tmp;

		tmp = a01; 
	    a01 = a10;
	    a10 = tmp;
	
	    tmp = a02;
	    a02 = a20;
	    a20 = tmp;
	
	    tmp = a03;
	    a03 = a30;
	    a30 = tmp;

		tmp = a12;
	    a12 = a21;
	    a21 = tmp;

		tmp = a13;
	    a13 = a31;
	    a31 = tmp;

		tmp = a23;
	    a23 = a32;
	    a32 = tmp;
	}
	
	void adjoint()
	{
		mat4 A;

		A.a00 =  det3x3(a11,a12,a13,a21,a22,a23,a31,a32,a33);
		A.a10 = -det3x3(a10,a12,a13,a20,a22,a23,a30,a32,a33);
		A.a20 =  det3x3(a10,a11,a13,a20,a21,a23,a30,a31,a33);
		A.a30 = -det3x3(a10,a11,a12,a20,a21,a22,a30,a31,a32);

		A.a01 = -det3x3(a01,a02,a03,a21,a22,a23,a31,a32,a33);
		A.a11 =  det3x3(a00,a02,a03,a20,a22,a23,a30,a32,a33);
		A.a21 = -det3x3(a00,a01,a03,a20,a21,a23,a30,a31,a33);
		A.a31 =  det3x3(a00,a01,a02,a20,a21,a22,a30,a31,a32);

		A.a02 =  det3x3(a01,a02,a03,a11,a12,a13,a31,a32,a33);
		A.a12 = -det3x3(a00,a02,a03,a10,a12,a13,a30,a32,a33);
		A.a22 =  det3x3(a00,a01,a03,a10,a11,a13,a30,a31,a33);
		A.a32 = -det3x3(a00,a01,a02,a10,a11,a12,a30,a31,a32);

		A.a03 = -det3x3(a01,a02,a03,a11,a12,a13,a21,a22,a23);
		A.a13 =  det3x3(a00,a02,a03,a10,a12,a13,a20,a22,a23);
		A.a23 = -det3x3(a00,a01,a03,a10,a11,a13,a20,a21,a23);
		A.a33 =  det3x3(a00,a01,a02,a10,a11,a12,a20,a21,a22);

		*this = A;
	}

	void adjoint_transpose()
	{
		mat4 A;

		A.a00 =  det3x3(a11,a12,a13,a21,a22,a23,a31,a32,a33);
		A.a10 = -det3x3(a01,a02,a03,a21,a22,a23,a31,a32,a33);
		A.a20 =  det3x3(a01,a02,a03,a11,a12,a13,a31,a32,a33);
		A.a30 = -det3x3(a01,a02,a03,a11,a12,a13,a21,a22,a23);

		A.a01 = -det3x3(a10,a12,a13,a20,a22,a23,a30,a32,a33);
		A.a11 =  det3x3(a00,a02,a03,a20,a22,a23,a30,a32,a33);
		A.a21 = -det3x3(a00,a02,a03,a10,a12,a13,a30,a32,a33);
		A.a31 =  det3x3(a00,a02,a03,a10,a12,a13,a20,a22,a23);

		A.a02 =  det3x3(a10,a11,a13,a20,a21,a23,a30,a31,a33);
		A.a12 = -det3x3(a00,a01,a03,a20,a21,a23,a30,a31,a33);
		A.a22 =  det3x3(a00,a01,a03,a10,a11,a13,a30,a31,a33);
		A.a32 = -det3x3(a00,a01,a03,a10,a11,a13,a20,a21,a23);

		A.a03 = -det3x3(a10,a11,a12,a20,a21,a22,a30,a31,a32);
		A.a13 =  det3x3(a00,a01,a02,a20,a21,a22,a30,a31,a32);
		A.a23 = -det3x3(a00,a01,a02,a10,a11,a12,a30,a31,a32);
		A.a33 =  det3x3(a00,a01,a02,a10,a11,a12,a20,a21,a22);

		*this = A;
	}

	void negate()
	{
		a00 = -a00; a01 = -a01; a02 = -a02; a03 = -a03;
		a10 = -a10; a11 = -a11; a12 = -a12; a13 = -a13;
		a20 = -a20; a21 = -a21; a22 = -a22; a23 = -a23;
		a30 = -a30; a31 = -a31; a32 = -a32; a33 = -a33;
	}

	void invert()
	{
		mat4 A;

		A.a00 =  det3x3(a11,a12,a13,a21,a22,a23,a31,a32,a33);
		A.a10 = -det3x3(a10,a12,a13,a20,a22,a23,a30,a32,a33);
		A.a20 =  det3x3(a10,a11,a13,a20,a21,a23,a30,a31,a33);
		A.a30 = -det3x3(a10,a11,a12,a20,a21,a22,a30,a31,a32);

		A.a01 = -det3x3(a01,a02,a03,a21,a22,a23,a31,a32,a33);
		A.a11 =  det3x3(a00,a02,a03,a20,a22,a23,a30,a32,a33);
		A.a21 = -det3x3(a00,a01,a03,a20,a21,a23,a30,a31,a33);
		A.a31 =  det3x3(a00,a01,a02,a20,a21,a22,a30,a31,a32);

		A.a02 =  det3x3(a01,a02,a03,a11,a12,a13,a31,a32,a33);
		A.a12 = -det3x3(a00,a02,a03,a10,a12,a13,a30,a32,a33);
		A.a22 =  det3x3(a00,a01,a03,a10,a11,a13,a30,a31,a33);
		A.a32 = -det3x3(a00,a01,a02,a10,a11,a12,a30,a31,a32);

		A.a03 = -det3x3(a01,a02,a03,a11,a12,a13,a21,a22,a23);
		A.a13 =  det3x3(a00,a02,a03,a10,a12,a13,a20,a22,a23);
		A.a23 = -det3x3(a00,a01,a03,a10,a11,a13,a20,a21,a23);
		A.a33 =  det3x3(a00,a01,a02,a10,a11,a12,a20,a21,a22);
	
		const Real det = a00*A.a00 + a01*A.a10 + a02*A.a20 + a03*A.a30;
		const Real oodet = (Real)1.0 / det;

		A *= oodet;

		*this = A;
	}

	Real det() const
	{
		return a00*det3x3(a11,a12,a13,a21,a22,a23,a31,a32,a33)
			 - a10*det3x3(a01,a02,a03,a21,a22,a23,a31,a32,a33)
			 + a20*det3x3(a01,a02,a03,a11,a12,a13,a31,a32,a33)
			 - a30*det3x3(a01,a02,a03,a11,a12,a13,a21,a22,a23);
	}

	inline Real trace() const
	{
		return a00 + a11 + a22 + a33;
	}

    inline vec4 operator[](const int i) const
    {
        return vec4(mat_array[i], mat_array[i+4], mat_array[i+8], mat_array[i+12]);
    }

    inline Real& operator() (const int i, const int j)
    {
        return mat_array[ j * 4 + i ];
    }

    inline const Real operator()(const int i, const int j) const
    {
        return  mat_array[ j * 4 + i ];
    }

	inline Real& operator[](const int i) { return mat_array[i]; }

	inline operator const Real *() const { return mat_array; }

    inline mat4 & operator*=(const Real & lambda)
    {
		a00 *= lambda; a01 *= lambda; a02 *= lambda; a03 *= lambda;
		a10 *= lambda; a11 *= lambda; a12 *= lambda; a13 *= lambda;
		a20 *= lambda; a21 *= lambda; a22 *= lambda; a23 *= lambda;
		a30 *= lambda; a31 *= lambda; a32 *= lambda; a33 *= lambda;

        return *this;
    }

    inline mat4 & operator-=(const mat4 & M)
    {
		a00 -= M.a00; a01 -= M.a01; a02 -= M.a02; a03 -= M.a03;
		a10 -= M.a10; a11 -= M.a11; a12 -= M.a12; a13 -= M.a13;
		a20 -= M.a20; a21 -= M.a21; a22 -= M.a22; a23 -= M.a23;
		a30 -= M.a30; a31 -= M.a31; a32 -= M.a32; a33 -= M.a33;

        return *this;
    }

    inline mat4 & operator+=(const mat4 & M)
    {
		a00 += M.a00; a01 += M.a01; a02 += M.a02; a03 += M.a03;
		a10 += M.a10; a11 += M.a11; a12 += M.a12; a13 += M.a13;
		a20 += M.a20; a21 += M.a21; a22 += M.a22; a23 += M.a23;
		a30 += M.a30; a31 += M.a31; a32 += M.a32; a33 += M.a33;

        return *this;
    }

	friend const vec4 operator*(const mat4& M, const vec4& v)
	{
		vec4 u;
	    u.x = M.a00 * v.x + M.a01 * v.y + M.a02 * v.z + M.a03 * v.w;
	    u.y = M.a10 * v.x + M.a11 * v.y + M.a12 * v.z + M.a13 * v.w;
	    u.z = M.a20 * v.x + M.a21 * v.y + M.a22 * v.z + M.a23 * v.w;
	    u.w = M.a30 * v.x + M.a31 * v.y + M.a32 * v.z + M.a33 * v.w;
	    return u;
	}

	friend const vec4 operator*(const vec4& v, const mat4& M)
	{
		vec4 u;
	    u.x = v.x * M.a00 + v.y * M.a10 + v.z * M.a20 + v.w * M.a30;
	    u.y = v.x * M.a01 + v.y * M.a11 + v.z * M.a21 + v.w * M.a31;
	    u.z = v.x * M.a02 + v.y * M.a12 + v.z * M.a22 + v.w * M.a32;
	    u.w = v.x * M.a03 + v.y * M.a13 + v.z * M.a23 + v.w * M.a33;
	    return u;
	}

	friend vec4 & mult(vec4& u, const mat4& M, const vec4& v)
	{
	    u.x = M.a00 * v.x + M.a01 * v.y + M.a02 * v.z + M.a03 * v.w;
	    u.y = M.a10 * v.x + M.a11 * v.y + M.a12 * v.z + M.a13 * v.w;
	    u.z = M.a20 * v.x + M.a21 * v.y + M.a22 * v.z + M.a23 * v.w;
	    u.w = M.a30 * v.x + M.a31 * v.y + M.a32 * v.z + M.a33 * v.w;
	    return u;
	}

	friend vec4 & mult(vec4& u, const vec4& v, const mat4& M)
	{
	    u.x = M.a00 * v.x + M.a10 * v.y + M.a20 * v.z + M.a30 * v.w;
	    u.y = M.a01 * v.x + M.a11 * v.y + M.a21 * v.z + M.a31 * v.w;
	    u.z = M.a02 * v.x + M.a12 * v.y + M.a22 * v.z + M.a32 * v.w;
	    u.w = M.a03 * v.x + M.a13 * v.y + M.a23 * v.z + M.a33 * v.w;
	    return u;
	}

	friend mat4 & add(mat4& A, const mat4& B)
	{
		A.a00 += B.a00;
		A.a10 += B.a10;
		A.a20 += B.a20;
		A.a30 += B.a30;
		A.a01 += B.a01;
		A.a11 += B.a11;
		A.a21 += B.a21;
		A.a31 += B.a31;
		A.a02 += B.a02;
		A.a12 += B.a12;
		A.a22 += B.a22;
		A.a32 += B.a32;
		A.a03 += B.a03;
		A.a13 += B.a13;
		A.a23 += B.a23;
		A.a33 += B.a33;
		return A;
	}

	friend mat4 & add(mat4 & C, const mat4 & A, const mat4 & B)
	{
		// If there is selfassignment involved
		// we can't go without a temporary.
		if (&C == &A || &C == &B)
		{
			mat4 mTemp;

			mTemp.a00 = A.a00 + B.a00;
			mTemp.a01 = A.a01 + B.a01;
			mTemp.a02 = A.a02 + B.a02;
			mTemp.a03 = A.a03 + B.a03;
			mTemp.a10 = A.a10 + B.a10;
			mTemp.a11 = A.a11 + B.a11;
			mTemp.a12 = A.a12 + B.a12;
			mTemp.a13 = A.a13 + B.a13;
			mTemp.a20 = A.a20 + B.a20;
			mTemp.a21 = A.a21 + B.a21;
			mTemp.a22 = A.a22 + B.a22;
			mTemp.a23 = A.a23 + B.a23;
			mTemp.a30 = A.a30 + B.a30;
			mTemp.a31 = A.a31 + B.a31;
			mTemp.a32 = A.a32 + B.a32;
			mTemp.a33 = A.a33 + B.a33;

			C = mTemp;
		}
		else
		{
			C.a00 = A.a00 + B.a00;
			C.a01 = A.a01 + B.a01;
			C.a02 = A.a02 + B.a02;
			C.a03 = A.a03 + B.a03;
			C.a10 = A.a10 + B.a10;
			C.a11 = A.a11 + B.a11;
			C.a12 = A.a12 + B.a12;
			C.a13 = A.a13 + B.a13;
			C.a20 = A.a20 + B.a20;
			C.a21 = A.a21 + B.a21;
			C.a22 = A.a22 + B.a22;
			C.a23 = A.a23 + B.a23;
			C.a30 = A.a30 + B.a30;
			C.a31 = A.a31 + B.a31;
			C.a32 = A.a32 + B.a32;
			C.a33 = A.a33 + B.a33;
		}
		return C;
	}

	// C = A * B

	friend mat4 & mult(mat4& C, const mat4& A, const mat4& B)
	{
                                // If there is self-assignment involved
                                // we can't go without a temporary.
		if (&C == &A || &C == &B)
		{
			mat4 mTemp;

			mTemp.a00 = A.a00 * B.a00 + A.a01 * B.a10 + A.a02 * B.a20 + A.a03 * B.a30;
			mTemp.a10 = A.a10 * B.a00 + A.a11 * B.a10 + A.a12 * B.a20 + A.a13 * B.a30;
			mTemp.a20 = A.a20 * B.a00 + A.a21 * B.a10 + A.a22 * B.a20 + A.a23 * B.a30;
			mTemp.a30 = A.a30 * B.a00 + A.a31 * B.a10 + A.a32 * B.a20 + A.a33 * B.a30;
			mTemp.a01 = A.a00 * B.a01 + A.a01 * B.a11 + A.a02 * B.a21 + A.a03 * B.a31;
			mTemp.a11 = A.a10 * B.a01 + A.a11 * B.a11 + A.a12 * B.a21 + A.a13 * B.a31;
			mTemp.a21 = A.a20 * B.a01 + A.a21 * B.a11 + A.a22 * B.a21 + A.a23 * B.a31;
			mTemp.a31 = A.a30 * B.a01 + A.a31 * B.a11 + A.a32 * B.a21 + A.a33 * B.a31;
			mTemp.a02 = A.a00 * B.a02 + A.a01 * B.a12 + A.a02 * B.a22 + A.a03 * B.a32;
			mTemp.a12 = A.a10 * B.a02 + A.a11 * B.a12 + A.a12 * B.a22 + A.a13 * B.a32;
			mTemp.a22 = A.a20 * B.a02 + A.a21 * B.a12 + A.a22 * B.a22 + A.a23 * B.a32;
			mTemp.a32 = A.a30 * B.a02 + A.a31 * B.a12 + A.a32 * B.a22 + A.a33 * B.a32;
			mTemp.a03 = A.a00 * B.a03 + A.a01 * B.a13 + A.a02 * B.a23 + A.a03 * B.a33;
			mTemp.a13 = A.a10 * B.a03 + A.a11 * B.a13 + A.a12 * B.a23 + A.a13 * B.a33;
			mTemp.a23 = A.a20 * B.a03 + A.a21 * B.a13 + A.a22 * B.a23 + A.a23 * B.a33;
			mTemp.a33 = A.a30 * B.a03 + A.a31 * B.a13 + A.a32 * B.a23 + A.a33 * B.a33;

			C = mTemp;
		}
		else
		{
			C.a00 = A.a00 * B.a00 + A.a01 * B.a10 + A.a02 * B.a20 + A.a03 * B.a30;
			C.a10 = A.a10 * B.a00 + A.a11 * B.a10 + A.a12 * B.a20 + A.a13 * B.a30;
			C.a20 = A.a20 * B.a00 + A.a21 * B.a10 + A.a22 * B.a20 + A.a23 * B.a30;
			C.a30 = A.a30 * B.a00 + A.a31 * B.a10 + A.a32 * B.a20 + A.a33 * B.a30;
			C.a01 = A.a00 * B.a01 + A.a01 * B.a11 + A.a02 * B.a21 + A.a03 * B.a31;
			C.a11 = A.a10 * B.a01 + A.a11 * B.a11 + A.a12 * B.a21 + A.a13 * B.a31;
			C.a21 = A.a20 * B.a01 + A.a21 * B.a11 + A.a22 * B.a21 + A.a23 * B.a31;
			C.a31 = A.a30 * B.a01 + A.a31 * B.a11 + A.a32 * B.a21 + A.a33 * B.a31;
			C.a02 = A.a00 * B.a02 + A.a01 * B.a12 + A.a02 * B.a22 + A.a03 * B.a32;
			C.a12 = A.a10 * B.a02 + A.a11 * B.a12 + A.a12 * B.a22 + A.a13 * B.a32;
			C.a22 = A.a20 * B.a02 + A.a21 * B.a12 + A.a22 * B.a22 + A.a23 * B.a32;
			C.a32 = A.a30 * B.a02 + A.a31 * B.a12 + A.a32 * B.a22 + A.a33 * B.a32;
			C.a03 = A.a00 * B.a03 + A.a01 * B.a13 + A.a02 * B.a23 + A.a03 * B.a33;
			C.a13 = A.a10 * B.a03 + A.a11 * B.a13 + A.a12 * B.a23 + A.a13 * B.a33;
			C.a23 = A.a20 * B.a03 + A.a21 * B.a13 + A.a22 * B.a23 + A.a23 * B.a33;
			C.a33 = A.a30 * B.a03 + A.a31 * B.a13 + A.a32 * B.a23 + A.a33 * B.a33;
		}

		return C;
	}

	friend mat4 & transpose(mat4& A)
	{
	    Real tmp;
		tmp = A.a01;
		A.a01 = A.a10;
		A.a10 = tmp;

		tmp = A.a02;
		A.a02 = A.a20;
		A.a20 = tmp;

		tmp = A.a03;
		A.a03 = A.a30;
		A.a30 = tmp;

		tmp = A.a12;
		A.a12 = A.a21;
		A.a21 = tmp;

		tmp = A.a13;
		A.a13 = A.a31;
		A.a31 = tmp;

		tmp = A.a23;
		A.a23 = A.a32;
		A.a32 = tmp;
		return A;
	}

	friend mat4 & transpose(mat4& B, const mat4& A)
	{
		B.a00 = A.a00;
		B.a01 = A.a10;
		B.a02 = A.a20;
		B.a03 = A.a30;
		B.a10 = A.a01;
		B.a11 = A.a11;
		B.a12 = A.a21;
		B.a13 = A.a31;
		B.a20 = A.a02;
		B.a21 = A.a12;
		B.a22 = A.a22;
		B.a23 = A.a32;
		B.a30 = A.a03;
		B.a31 = A.a13;
		B.a32 = A.a23;
		B.a33 = A.a33;
		return B;
	}

	friend Real det(const mat4& A)
	{
		return A.det();
	}

	friend Real trace(const mat4& A)
	{
		return A.trace();
	}

	friend mat4 & invert(mat4& B, const mat4& A)
	{
		B.a00 =  det3x3(A.a11,A.a12,A.a13,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a10 = -det3x3(A.a10,A.a12,A.a13,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a20 =  det3x3(A.a10,A.a11,A.a13,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a30 = -det3x3(A.a10,A.a11,A.a12,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);

		B.a01 = -det3x3(A.a01,A.a02,A.a03,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a11 =  det3x3(A.a00,A.a02,A.a03,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a21 = -det3x3(A.a00,A.a01,A.a03,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a31 =  det3x3(A.a00,A.a01,A.a02,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);

		B.a02 =  det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a31,A.a32,A.a33);
		B.a12 = -det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a30,A.a32,A.a33);
		B.a22 =  det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a30,A.a31,A.a33);
		B.a32 = -det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a30,A.a31,A.a32);

		B.a03 = -det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a21,A.a22,A.a23);
		B.a13 =  det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a20,A.a22,A.a23);
		B.a23 = -det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a20,A.a21,A.a23);
		B.a33 =  det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a20,A.a21,A.a22);
	
		const Real det = A.a00*B.a00 + A.a01*B.a10 + A.a02*B.a20 + A.a03*B.a30;
		const Real oodet = (Real)1.0 / det;

		B *= oodet;

		return B;
	}

	friend mat4& adjoint(mat4& A)
	{
		mat4 B;

		B.a00 =  det3x3(A.a11,A.a12,A.a13,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a10 = -det3x3(A.a10,A.a12,A.a13,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a20 =  det3x3(A.a10,A.a11,A.a13,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a30 = -det3x3(A.a10,A.a11,A.a12,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);

		B.a01 = -det3x3(A.a01,A.a02,A.a03,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a11 =  det3x3(A.a00,A.a02,A.a03,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a21 = -det3x3(A.a00,A.a01,A.a03,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a31 =  det3x3(A.a00,A.a01,A.a02,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);

		B.a02 =  det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a31,A.a32,A.a33);
		B.a12 = -det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a30,A.a32,A.a33);
		B.a22 =  det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a30,A.a31,A.a33);
		B.a32 = -det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a30,A.a31,A.a32);

		B.a03 = -det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a21,A.a22,A.a23);
		B.a13 =  det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a20,A.a22,A.a23);
		B.a23 = -det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a20,A.a21,A.a23);
		B.a33 =  det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a20,A.a21,A.a22);

		A = B;
		return A;
	}

	friend mat4& adjoint(mat4& B, const mat4& A)
	{
		B.a00 =  det3x3(A.a11,A.a12,A.a13,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a10 = -det3x3(A.a10,A.a12,A.a13,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a20 =  det3x3(A.a10,A.a11,A.a13,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a30 = -det3x3(A.a10,A.a11,A.a12,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);

		B.a01 = -det3x3(A.a01,A.a02,A.a03,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a11 =  det3x3(A.a00,A.a02,A.a03,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a21 = -det3x3(A.a00,A.a01,A.a03,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a31 =  det3x3(A.a00,A.a01,A.a02,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);

		B.a02 =  det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a31,A.a32,A.a33);
		B.a12 = -det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a30,A.a32,A.a33);
		B.a22 =  det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a30,A.a31,A.a33);
		B.a32 = -det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a30,A.a31,A.a32);

		B.a03 = -det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a21,A.a22,A.a23);
		B.a13 =  det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a20,A.a22,A.a23);
		B.a23 = -det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a20,A.a21,A.a23);
		B.a33 =  det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a20,A.a21,A.a22);

		return B;
	}

	friend mat4& adjoint_transpose(mat4& A)
	{
		mat4 B;

		B.a00 =  det3x3(A.a11,A.a12,A.a13,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a10 = -det3x3(A.a01,A.a02,A.a03,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a20 =  det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a31,A.a32,A.a33);
		B.a30 = -det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a21,A.a22,A.a23);

		B.a01 = -det3x3(A.a10,A.a12,A.a13,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a11 =  det3x3(A.a00,A.a02,A.a03,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a21 = -det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a30,A.a32,A.a33);
		B.a31 =  det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a20,A.a22,A.a23);

		B.a02 =  det3x3(A.a10,A.a11,A.a13,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a12 = -det3x3(A.a00,A.a01,A.a03,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a22 =  det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a30,A.a31,A.a33);
		B.a32 = -det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a20,A.a21,A.a23);

		B.a03 = -det3x3(A.a10,A.a11,A.a12,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);
		B.a13 =  det3x3(A.a00,A.a01,A.a02,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);
		B.a23 = -det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a30,A.a31,A.a32);
		B.a33 =  det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a20,A.a21,A.a22);

		A = B;
		return A;
	}

	friend mat4& adjoint_transpose(mat4& B, const mat4& A)
	{
		B.a00 =  det3x3(A.a11,A.a12,A.a13,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a10 = -det3x3(A.a01,A.a02,A.a03,A.a21,A.a22,A.a23,A.a31,A.a32,A.a33);
		B.a20 =  det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a31,A.a32,A.a33);
		B.a30 = -det3x3(A.a01,A.a02,A.a03,A.a11,A.a12,A.a13,A.a21,A.a22,A.a23);

		B.a01 = -det3x3(A.a10,A.a12,A.a13,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a11 =  det3x3(A.a00,A.a02,A.a03,A.a20,A.a22,A.a23,A.a30,A.a32,A.a33);
		B.a21 = -det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a30,A.a32,A.a33);
		B.a31 =  det3x3(A.a00,A.a02,A.a03,A.a10,A.a12,A.a13,A.a20,A.a22,A.a23);

		B.a02 =  det3x3(A.a10,A.a11,A.a13,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a12 = -det3x3(A.a00,A.a01,A.a03,A.a20,A.a21,A.a23,A.a30,A.a31,A.a33);
		B.a22 =  det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a30,A.a31,A.a33);
		B.a32 = -det3x3(A.a00,A.a01,A.a03,A.a10,A.a11,A.a13,A.a20,A.a21,A.a23);

		B.a03 = -det3x3(A.a10,A.a11,A.a12,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);
		B.a13 =  det3x3(A.a00,A.a01,A.a02,A.a20,A.a21,A.a22,A.a30,A.a31,A.a32);
		B.a23 = -det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a30,A.a31,A.a32);
		B.a33 =  det3x3(A.a00,A.a01,A.a02,A.a10,A.a11,A.a12,A.a20,A.a21,A.a22);

		return B;
	}

	friend mat4 & negate(mat4 & M)
	{
		M.a00 = -M.a00; M.a01 = -M.a01; M.a02 = -M.a02; M.a03 = -M.a03;
		M.a10 = -M.a10; M.a11 = -M.a11; M.a12 = -M.a12; M.a13 = -M.a13;
		M.a20 = -M.a20; M.a21 = -M.a21; M.a22 = -M.a22; M.a23 = -M.a23;
		M.a30 = -M.a30; M.a31 = -M.a31; M.a32 = -M.a32; M.a33 = -M.a33;

		return M;
	}

};


// quaternion
struct quat {
	union {
		struct {
			Real x, y, z, w;
		};
		Real comp[4];
	};

	quat() {};
	~quat() {};

	quat(const Real _x, const Real _y, const Real _z, const Real _w)
		: x(_x), y(_y), z(_z), w(_w) {};
	quat(const quat& q) : x(q.x), y(q.y), z(q.z), w(q.w) {};

	quat(const vec3& axis, Real angle)
	{
		const Real len = axis.norm();
		if (len) {
			const Real invLen = 1 / len;
			const Real angle2 = angle / 2;
			const Real scale = sinf(angle2) * invLen;
			x = scale * axis[0];
			y = scale * axis[1];
			z = scale * axis[2];
			w = cosf(angle2);
		}
	}

	quat(const mat3& rot)
	{
		fromMatrix(rot);
	}

	inline void set(const Real _x, const Real _y, const Real _z, const Real _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	inline void set(const quat& q) {
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
	}

	inline void mul(const quat& q) {
	    // store on stack for aliasing-safty
	    set(
	        x*q.w + w*q.x + y*q.z - z*q.y,
	        y*q.w + w*q.y + z*q.x - x*q.z,
	        z*q.w + w*q.z + x*q.y - y*q.x,
	        w*q.w - x*q.x - y*q.y - z*q.z
	        );
	}

	inline Real dot(const quat& q) const
	{
	    return x*q.x + y*q.y + z*q.z + w*q.w;
	}

	inline float norm() const {
		return x*x + y*y + z*z + w*w;
	}

    inline quat inverse() const
	{
		return quat(- x, - y, - z, w);
	}

	void normalize()
	{
		const Real len = sqrtf(x*x + y*y + z*z + w*w);
		x /= len;
		y /= len;
		z /= len;
		w /= len;
	}

	void getMatrix(float m[]) const {
	 	const float s = 2.0f / norm();
		m[0] = 1 - s*(y*y+z*z);
		m[1] = s*(x*y-w*z);
		m[2] = s*(x*z+w*y);
		m[3] = 0.0f;
		m[4] = s*(x*y+w*z);
		m[5] = 1 - s*(x*x+z*z);
		m[6] = s*(y*z-w*x);
		m[7] = 0.0f;
		m[8] = s*(x*z-w*y);
		m[9] = s*(y*z+w*x);
		m[10] = 1 - s*(x*x+y*y);
		m[11] = 0.0f;
		m[12] = 0.0f;
		m[13] = 0.0f;
		m[14] = 0.0f;
		m[15] = 1.0f;
	}

	void fromMatrix(const mat3& mat)
	{
		const Real trace = mat(0, 0) + mat(1, 1) + mat(2, 2);
		if (trace > 0.0) 
	    {
			Real scale = sqrtf(trace + (Real)1.0);
			w = (Real)0.5 * scale;
			scale = (Real)0.5 / scale;
			x = scale * (mat(2, 1) - mat(1, 2));
			y = scale * (mat(0, 2) - mat(2, 0));
			z = scale * (mat(1, 0) - mat(0, 1));
		}
		else 
	    {
			static int next[] = { 1, 2, 0 };
			int i = 0;
			if (mat(1, 1) > mat(0, 0))
				i = 1;
			if (mat(2, 2) > mat(i, i))
				i = 2;
			int j = next[i];
			int k = next[j];
			Real scale = sqrtf(mat(i, i) - mat(j, j) - mat(k, k) + 1);
			Real* q[] = { &x, &y, &z };
			*q[i] = (Real)0.5 * scale;
			scale = (Real)0.5 / scale;
			w = scale * (mat(k, j) - mat(j, k));
			*q[j] = scale * (mat(j, i) + mat(i, j));
			*q[k] = scale * (mat(k, i) + mat(i, k));
		}
	}

	void toMatrix(mat3& mat) const
	{
		const Real x2 = x * 2;
		const Real y2 = y * 2;
		const Real z2 = z * 2;
		const Real wx = x2 * w;
		const Real wy = y2 * w;
		const Real wz = z2 * w;
		const Real xx = x2 * x;
		const Real xy = y2 * x;
		const Real xz = z2 * x;
		const Real yy = y2 * y;
		const Real yz = z2 * y;
		const Real zz = z2 * z;
		mat(0, 0) = 1 - (yy + zz);
		mat(0, 1) = xy - wz;
		mat(0, 2) = xz + wy;
		mat(1, 0) = xy + wz;
		mat(1, 1) = 1 - (xx + zz);
		mat(1, 2) = yz - wx;
		mat(2, 0) = xz - wy;
		mat(2, 1) = yz + wx;
		mat(2, 2) = 1 - (xx + yy);
	}

	quat& operator=(const quat& quat)
	{
		x = quat.x;
		y = quat.y;
		z = quat.z;
		w = quat.w;
		return *this;
	}

	inline quat operator-()
	{
		return quat(-x, -y, -z, -w);
	}

	quat& operator*=(const quat& q)
	{
		const Real _x = w * q.x + x * q.w + y * q.z - z * q.y;
		const Real _y = w * q.y + y * q.w + z * q.x - x * q.z;
		const Real _z = w * q.z + z * q.w + x * q.y - y * q.x;
		const Real _w = w * q.w - x * q.x - y * q.y - z * q.z;
		x = _x;
		y = _y;
		z = _z;
		w = _w;
		return *this;
	}

	Real& operator[](const int i) { return comp[i]; }

	const Real operator[](const int i) const { return comp[i]; }

	friend mat3 & quat2mat(mat3& M, const quat& q)
	{
		q.toMatrix(M);
	    return M;
	}

	friend quat & mat2quat(quat& q, const mat3& M)
	{
		q.fromMatrix(M);
	    return q;
	} 

	/*
	    Given an axis and angle, compute quaternion.
	 */
	friend quat & axis2quat(quat& q, const vec3& a, const Real phi)
	{
	    vec3 tmp(a);

		tmp.normalize();
		const Real s = sinf(phi/(Real)2.0);
	    q.x = s * tmp.x;
	    q.y = s * tmp.y;
	    q.z = s * tmp.z;
	    q.w = cosf(phi/(Real)2.0);
	    return q;
	}

	friend quat & conj(quat & p)
	{
	    p.x = -p.x;
	    p.y = -p.y;
	    p.z = -p.z;
	    return p;
	}

	friend quat & conj(quat& p, const quat& q)
	{
	    p.x = -q.x;
	    p.y = -q.y;
	    p.z = -q.z;
	    p.w = q.w;
	    return p;
	}

	friend quat & add_quats(quat& p, const quat& q1, const quat& q2)
	{
	    quat t1, t2;
	
	    t1 = q1;
	    t1.x *= q2.w;
	    t1.y *= q2.w;
	    t1.z *= q2.w;
	
	    t2 = q2;
	    t2.x *= q1.w;
	    t2.y *= q1.w;
	    t2.z *= q1.w;
	
	    p.x = (q2.y * q1.z) - (q2.z * q1.y) + t1.x + t2.x;
	    p.y = (q2.z * q1.x) - (q2.x * q1.z) + t1.y + t2.y;
	    p.z = (q2.x * q1.y) - (q2.y * q1.x) + t1.z + t2.z;
	    p.w = q1.w * q2.w - (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z);
	
	    return p;
	}

	friend Real dot(const quat& q1, const quat& q2)
	{
	    return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;
	}

	friend quat & slerp_quats(quat & p, Real s, const quat & q1, const quat & q2)
	{
	    Real cosine = q1.dot(q2);
		if (cosine < -1)
			cosine = -1;
		else if (cosine > 1)
			cosine = 1;
		const Real angle = (Real)acosf(cosine);
	    if (fabs(angle) < my_eps) {
			p = q1;
	        return p;
		}
	    const Real sine = sinf(angle);
	    const Real sineInv = 1.0f / sine;
	    const Real c1 = sinf((1.0f - s) * angle) * sineInv;
	    const Real c2 = sinf(s * angle) * sineInv;
		p.x = c1 * q1.x + c2 * q2.x;
		p.y = c1 * q1.y + c2 * q2.y;
		p.z = c1 * q1.z + c2 * q2.z;
		p.w = c1 * q1.w + c2 * q2.w;
	    return p;
	}

	friend const quat operator*(const quat& p, const quat& q)
	{
		return quat(
			p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
			p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z,
			p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x,
			p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z
		);
	}

	inline friend std::ostream& operator<<(std::ostream &o, const quat& q)
	{
		return o << "(" << q.x << "," << q.y << "," << q.z << "," << q.w << ")";
	}
};

inline vec3 & vec3::lmult(const mat3& M)
{
	const Real _x = M.a00 * x + M.a01 * y + M.a02 * z;
	const Real _y = M.a10 * x + M.a11 * y + M.a12 * z;
	const Real _z = M.a20 * x + M.a21 * y + M.a22 * z;
	x = _x; y = _y; z = _z;
	return *this;
}

inline vec3 & vec3::rmult(const mat3& M)
{
    const Real _x = x * M.a00 + y * M.a10 + z * M.a20;
    const Real _y = x * M.a01 + y * M.a11 + z * M.a21;
    const Real _z = x * M.a02 + y * M.a12 + z * M.a22;
	x = _x; y = _y; z = _z;
    return *this;
}

static const Real array9_id[] =         { 1.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f};

static const quat Identity(0, 0, 0, 1);

static const vec3      vec3_one(1.0f,1.0f,1.0f);
static const vec3      vec3_null(0.0f,0.0f,0.0f);
static const vec3      vec3_x(1.0f,0.0f,0.0f);
static const vec3      vec3_y(0.0f,1.0f,0.0f);
static const vec3      vec3_z(0.0f,0.0f,1.0f);
static const vec3      vec3_neg_x(-1.0f,0.0f,0.0f);
static const vec3      vec3_neg_y(0.0f,-1.0f,0.0f);
static const vec3      vec3_neg_z(0.0f,0.0f,-1.0f);
static const mat3      mat3_id(array9_id);

}

#endif // _MY_ALGEBRA_H_


