//#include "common.h"
//struct Color
//{
//	float r, g, b;
//};
//
//// OPERATOR OVERLOADING
//Color operator*(Color const& color, float scalar)
//{
//	return { color.r * scalar, color.g * scalar, color.b * scalar };
//}
//
//Color operator+(Color const& color, Color const& other)
//{
//	return { color.r + other.r, color.g + other.g, color.b + other.b };
//}
//
//Color operator*(float scalar, Color const& color)
//{
//	return { scalar * color.r, scalar * color.g, scalar * color.b };
//}
//// END OPERATOR OVERLOADING
//
//Color getAlphaCompositedColor(Color const& foregroundColor, Color const& backgroundColor, float alphaValue)
//{
//	return alphaValue * foregroundColor + ((1.0f - alphaValue) * backgroundColor);
//}
//
//// RAYTRACING
////
////A basic ray tracer therefore has three parts :
////1. ray generation, which computes the origin and direction of each pixel’s
////viewing ray based on the camera geometry;
////2. ray intersection, which finds the closest object intersecting the viewing ray;
////3. shading, which computes the pixel color based on the results of ray intersection.
//
////for each pixel do
////	compute viewing ray
////	find first object hit by ray and its surface normal n
////	set pixel color to value computed from hit point, light, and n
//
////
////the basic tools of ray generation are the viewpoint(or
////	view direction, for parallel views) and the image plane.There are many ways to
////	work out the details of camera geometry; in this section we explain one based
////	on orthonormal bases that supports normaland oblique paralleland orthographic
////	views.
////	In order to generate rays, we first need a mathematical representation for a ray.
////	A ray is really just an origin pointand a propagation direction; a 3D parametric
////	line is ideal for this.As discussed in Section 2.5.7, the 3D parametric line from
////	the eye e to a point s on the image plane(Figure 4.6) is given by
////					p(t) = e + t(s − e).
//
//struct vec3
//{
//	float x, y, z;
//};
//
//vec3 operator+(vec3 const& first, vec3 const& second)
//{
//	return { first.x + second.x, first.y + second.y, first.z + second.z };
//}
//
//vec3 operator-(vec3 const& first, vec3 const& second)
//{
//	return { first.x - second.x, first.y - second.y, first.z - second.z };
//}
//
//vec3 operator*(float scalar, vec3 const& vector)
//{
//	return { scalar * vector.x, scalar * vector.y, scalar * vector.z };
//}
//
//vec3 operator*(vec3 const& vector, unsigned int scalar)
//{
//	float _scalar = (float)scalar;
//	return { _scalar * vector.x, _scalar * vector.y, _scalar * vector.z };
//}
//
//vec3 operator/(vec3 const& vector, unsigned int scalar)
//{
//	return { scalar * vector.x, scalar * vector.y, scalar * vector.z };
//}
//
//// p(t) = e + t(s − e).
//// p = return value of getRay()
//// t = fractionalDistance
//// s = rayDestiny
//// e = rayOrigin
//vec3 getRay(float fractionalDistance, vec3 const& rayOrigin, vec3 const& rayDestiny)
//{
//	return rayOrigin + fractionalDistance * (rayDestiny - rayOrigin);
//}
//
//struct textureCoordinate
//{
//	vec3 u, v;
//};
//
//struct Rectangle
//{
//	vec3 right, left, bottom, top;
//};
//
//struct imageDimension
//{
//	uint32_t width, height;
//};
//
//struct Pixel
//{
//	uint32_t x, y;
//};
//
//struct Ray
//{
//	vec3 origin;
//	vec3 direction;
//};
//
//textureCoordinate getTextureCoordinatesFromPixelImage(Rectangle const& rect, imageDimension const& imgDimension, Pixel const& pixel)
//{
//	textureCoordinate u_v;
//	u_v.u = rect.left + (rect.right - rect.left) * (pixel.x + 0.5) / imgDimension.width;
//	u_v.u = rect.bottom + (rect.top - rect.bottom) * (pixel.y + 0.5) / imgDimension.height;
//
//	return u_v;
//}
//
//void generateOrtographicViewingRays()
//{
//	// compute u and v using (4.1)
//	// ray.direction ←− w ----------- w == image plane normal
//	// ray.origin← e + u_k * u + v_k * v
//}
//
//void generatePerspectiveOrtographicViewingRays()
//{
//	// compute uand v using (4.1)
//	// ray.direction ← −dw + u_kursive * u + v_kursive * v
//	// ray.origin ← e
//}