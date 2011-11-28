/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Vector2D.hpp
 *
 *  Created on: Jun 29, 2011
 *      Author: linbo
 */

#pragma once

#include <cmath>

namespace sim_mob{

        /**
         * Class template to implement 2-dimensional vector with x and y co-ordinates
         * of template parameter type.
         *
         * The textbook 2-dimensional vector is implemented by this class methods and
         * non-class functions.
         *
         * \sa operator+(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
         * \sa operator-(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
         * \sa operator*(const U& s, const Vector2D<T>& v)
         * \sa operator*(const Vector2D<T>& v, const U& s)
         * \sa operator/(const Vector2D<T>& v, const U& s)
         * \sa operator==(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
         * \sa operator!=(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
         * \sa operator-(const Vector2D<T>& v)
         * \sa operator*(const Vector2D<T>& vec1, const Vector2D<T>& vec2)
         * \sa length(const Vector2D<T>& vector)
         * \sa normalize(const Vector2D<T>& vector)
         */
        template< typename T >
	class Vector2D{

	private:
		T x;
		T y;
	public:
            /**
             * Default constructor.
             *
             * Creates a Vector2D at the origin.
             */
	    inline Vector2D() : x(T()), y(T()){}

            /**
             * Constructor.
             *
             * Creates a Vector2D at the specified \c x and \c y co-ordinates.
             */
	    inline Vector2D(const T& x, const T& y) : x(x), y(y){}

            /**
             * Copy constructor.
             *
             * Creates a Vector2D that has the same x and y values as the specified \c vector.
             */
	    inline Vector2D(const Vector2D& vector) : x(vector.getX()), y(vector.getY()){}

            /**
             * Destructor.
             */
	    inline ~Vector2D(){}

            /** Returns the x value.  */
	    inline T getX() const {
	      return x;
	    }
            /** Returns the y value.  */
	    inline T getY() const
	    {
	      return y;
	    }

            /** Sets the x value.  */
            void setX(const T& x)
            {
                this->x = x;
            }
            /** Sets the y value.  */
            void setY(const T& y)
            {
                this->y = y;
            }

            /**
             * Copy assignment.
             *
             * Sets this object to have the same x and y values as the specified \c vector.
             *
             * The return value is a const to prevent
             *     \code
             *     (a = b) = c;
             *     \endcode
             */
            const Vector2D& operator=(const Vector2D& vector)
            {
                if (&vector != this)
                {
                    x = vector.getX();
                    y = vector.getY();
                }
                return *this;
            }

	    /**
             * Add another vector to this vector.
             *
	     * Equivalent to <tt>*this = *this + vector</tt>
             * \sa operator+(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
             *
             * The return value is a const to prevent
             *     \code
             *     (a += b) = c;
             *     \endcode
	     */
	    inline const Vector2D& operator+=(const Vector2D& vector)
	    {
                x += vector.getX();
                y += vector.getY();
                return *this;
	    }

	    /**
             * Subtract another vector to this vector.
             *
	     * Equivalent to <tt>*this = *this - vector</tt>
             * \sa operator-(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
             *
             * The return value is a const to prevent
             *     \code
             *     (a -= b) = c;
             *     \endcode
	     */
	    inline const Vector2D& operator-=(const Vector2D& vector)
	    {
                x -= vector.getX();
                y -= vector.getY();
                return *this;
	    }

	    /**
             * Expand this vector by the scalar factor \c s.
             *
             * Equivalent to <tt>*this = s * *this</tt>.
             * \sa operator*(const U& s, const Vector2D<T>& v)
             * \sa operator*(const Vector2D<T>& v, const U& s)
             *
             * The return value is a const to prevent
             *     \code
             *     (vec1 *= s) = vec2;
             *     \endcode
	     */
            template< typename U >
	    inline Vector2D& operator*=(const U& s)
	    {
                x *= s;
                y *= s;
                return *this;
	    }

	    /**
             * Contract this vector by the scalar factor \c s.
             *
             * Equivalent to <tt>*this = *this / s</tt>.
             * \sa operator/(const Vector2D<T>& v, const U& s)
             *
             * The return value is a const to prevent
             *     \code
             *     (vec1 /= s) = vec2;
             *     \endcode
	     */
            template< typename U >
	    inline Vector2D& operator/=(const U& s)
	    {
                x /= s;
                y /= s;
                return *this;
	    }
	};

        /**
         *  Computes the vector sum of two vectors
         *
         *  The return value is a const to prevent
         *      \code
         *      (a + b) = c;
         *      \endcode
         */
        template< typename T >
        inline const Vector2D<T> operator+(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
        {
            return Vector2D<T>(lhs) += rhs;
        }

        /**
         *  Computes the vector difference of two vectors
         *
         *  The return value is a const to prevent
         *      \code
         *      (a - b) = c;
         *      \endcode
         */
        template< typename T >
        inline const Vector2D<T> operator-(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
        {
            return Vector2D<T>(lhs) -= rhs;
        }

        /**
         *  Computes the scalar multiplication of the specified \c vector
         *
         *  The return value is a const to prevent
         *      \code
         *      (s * vec1) = vec2;
         *      \endcode
         */
        template< typename T, typename U>
        inline const Vector2D<T> operator*(const U& s, const Vector2D<T>& v)
        {
            return Vector2D<T>(v) *= s;
        }
        /**
         *  Computes the scalar multiplication of the specified \c vector
         *
         *  The return value is a const to prevent
         *      \code
         *      (vec1 * s) = vec2;
         *      \endcode
         */
        template< typename T, typename U>
        inline const Vector2D<T> operator*(const Vector2D<T>& v, const U& s)
        {
            return Vector2D<T>(v) *= s;
        }

        /**
         *  Computes the scalar division of the specified \c vector
         *
         *  The return value is a const to prevent
         *      \code
         *      (vec1 / s) = vec2;
         *      \endcode
         */
        template< typename T, typename U>
        inline const Vector2D<T> operator/(const Vector2D<T>& v, const U& s)
        {
            return Vector2D<T>(v) /= s;
        }

        /**
         * unary - operator.
         *
         * Returns the inverse of the specified vector.
         *
         * The return value is a const to prevent
         *     \code
         *     -a = b;
         *     \endcode
         */
        template< typename T >
        inline const Vector2D<T> operator-(const Vector2D<T>& v)
        {
            return Vector2D<T>(-v.getX(), -v.getY());
        }

        /**
         *  Computes the dot product of two vectors
         */
        template< typename T >
        inline T operator*(const Vector2D<T>& vec1, const Vector2D<T>& vec2)
        {
            return (vec1.getX() * vec2.getX()) + (vec1.getY() * vec2.getY());
        }

        /**
         * Returns true if \c lhs and \c rhs have the same x and y values.
         */
        template< typename T >
        inline bool operator==(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
        {
            return (lhs.getX() == rhs.getX()) && (lhs.getY() == rhs.getY()); 
        }
        /**
         * Returns true if \c lhs and \c rhs do not have the same x and y values.
         */
        template< typename T >
        inline bool operator!=(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
        {
            return !operator==(lhs, rhs);
        }

    /**
     *  Computes the length of the specified \c vector
     */
        template< typename T >
	inline float length(const Vector2D<T>& vector)
	{
	return std::sqrt(vector * vector);
	}

    /**
     *  Computes the normalization of the specified \c vector
     *
     *  Returns a Vector2D that has unit length and that is parallel and in the same direction
     *  as the specfied \c vector .
     */
        template< typename T >
	inline Vector2D<T> normalize(const Vector2D<T>& vector)
	{
	return vector / length(vector);
	}

}
