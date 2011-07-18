/*
 * Vector2D.hpp
 *
 *  Created on: Jun 29, 2011
 *      Author: linbo
 */

#pragma once

#include <cmath>

namespace sim_mob{

	class Vector2D{

	private:
		float x;
		float y;
	public:
	    inline Vector2D() : x(0.0f), y(0.0f){}
	    inline Vector2D(float x, float y) : x(x), y(y){}
	    inline Vector2D(const Vector2D& vector) : x(vector.getX()), y(vector.getY()){}
	    inline ~Vector2D(){}

	    inline float getX() const {
	      return x;
	    }
	    inline float getY() const
	    {
	      return y;
	    }
	    /*!
	     *  Computes the vector sum of two vectors
	     */
	    inline Vector2D operator+(const Vector2D& vector) const
	    {
	      return Vector2D(x + vector.getX(), y + vector.getY());
	    }

	    /*!
	     *  Computes the vector difference of two vectors
	     */
	    inline Vector2D operator-(const Vector2D& vector) const
	    {
	      return Vector2D(x - vector.getX(), y - vector.getY());
	    }

	    /*!
	     *  Computes the dot product of two vectors
	     */
	    inline float operator*(const Vector2D& vector) const
	    {
	      return x * vector.getX() + y* vector.getY();
	    }
	    /*!
	     *  Computes the scalar multiplication of this vector
	     */
	    inline Vector2D operator*(float s) const
	    {
	      return Vector2D(x * s, y * s);
	    }

	    /*!
	     *  Computes the scalar division of this vector
	     */
	    inline Vector2D operator/(float s) const
	    {
	      const float div = 1.0f / s;

	      return Vector2D(x * div, y * div);
	    }
	};

    /*!
     *  Computes the length of specified vector
     */
	inline float length(const Vector2D& vector)
	{
	return std::sqrt(vector * vector);
	}

    /*!
     *  Computes the normalization of specified vector
     */
	inline Vector2D normalize(const Vector2D& vector)
	{
	return vector / length(vector);
	}

}
