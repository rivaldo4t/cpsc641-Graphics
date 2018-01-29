import numpy as np
import cv2
import math

file_name = 'checkered_512x512'
ext = '.png'
img = cv2.imread(file_name + ext)
img2 = cv2.imread(file_name + '_2' + ext)
img4 = cv2.imread(file_name + '_4' + ext)
img8 = cv2.imread(file_name + '_8' + ext)
img16 = cv2.imread(file_name + '_16' + ext)
img32 = cv2.imread(file_name + '_32' + ext)
img64 = cv2.imread(file_name + '_64' + ext)
img128 = cv2.imread(file_name + '_128' + ext)
img256 = cv2.imread(file_name + '_256' + ext)
img512 = cv2.imread(file_name + '_512' + ext)

class Scene:
    """
    The scene that gets rendered. Contains information like the camera
    position, the different objects present, etc.
    """

    def __init__(self, camera, objects, lights, width, height):
        self.camera = camera
        self.objects = objects
        self.lights = lights
        self.width = width
        self.height = height

    def render(self):
        """
        Return a `self.height`x`self.width` 2D array of `Color`s representing
        the color of each pixel, obtained via ray-tracing.
        """

        pixels = [
            [Color() for _ in range(self.width)] for _ in range(self.height)]

        for y in range(self.height):
            for x in range(self.width):
                ray_direction = Point(x, y) - self.camera
                ray = Ray(self.camera, ray_direction)

                # For Anisotropic Tri-linear Interpolation
                ray_direction1 = Point(x-0.5, y-0.5) - self.camera
                ray1 = Ray(self.camera, ray_direction1)
                ray_direction2 = Point(x+0.5, y-0.5) - self.camera
                ray2 = Ray(self.camera, ray_direction2)
                ray_direction3 = Point(x+0.5, y+0.5) - self.camera
                ray3 = Ray(self.camera, ray_direction3)
                ray_direction4 = Point(x-0.5, y+0.5) - self.camera
                ray4 = Ray(self.camera, ray_direction4)

                pixels[y][x] = self._trace_ray(ray, ray1, ray2, ray3, ray4)

        return pixels

    def _trace_ray(self, ray, ray1, ray2, ray3, ray4, depth=0, max_depth=5):
        """
        Recursively trace a ray through the scene, returning the color it
        accumulates.
        """

        color = Color()

        if depth >= max_depth:
            return color

        intersection = self._get_intersection(ray)
        if intersection is None:
            return color
        # '''
        # Map corners for each pixel
        intersection1 = self._get_intersection(ray1)
        if intersection1 is None:
            return color

        intersection2 = self._get_intersection(ray2)
        if intersection2 is None:
            return color

        intersection3 = self._get_intersection(ray3)
        if intersection3 is None:
            return color

        intersection4 = self._get_intersection(ray4)
        if intersection4 is None:
            return color
        # '''

        obj, dist = intersection
        # '''
        # Pixel Corner Distance
        obj, dist1 = intersection1
        obj, dist2 = intersection2
        obj, dist3 = intersection3
        obj, dist4 = intersection4
        # '''

        point_on_plane = ray.origin   + dist*ray.direction
        # '''
        # Pixel Corners on the 3D plane
        point_on_plane1 = ray1.origin + dist1 * ray1.direction
        point_on_plane2 = ray2.origin + dist2 * ray2.direction
        point_on_plane3 = ray3.origin + dist3 * ray3.direction
        point_on_plane4 = ray4.origin + dist4 * ray4.direction
        # '''

        imgx = point_on_plane.x
        imgy = np.sqrt(point_on_plane.y * point_on_plane.y + point_on_plane.z * point_on_plane.z)

        # '''
        # Pixel corners projected on Projection plane
        imgx1 = point_on_plane1.x
        imgy1 = np.sqrt(point_on_plane1.y * point_on_plane1.y + point_on_plane1.z * point_on_plane1.z)

        imgx2 = point_on_plane2.x
        imgy2 = np.sqrt(point_on_plane2.y * point_on_plane2.y + point_on_plane2.z * point_on_plane2.z)

        imgx3 = point_on_plane3.x
        imgy3 = np.sqrt(point_on_plane3.y * point_on_plane3.y + point_on_plane3.z * point_on_plane3.z)

        imgx4 = point_on_plane4.x
        imgy4 = np.sqrt(point_on_plane4.y * point_on_plane4.y + point_on_plane4.z * point_on_plane4.z)

        # Mid points of the quadrilateral formed by pixel cordinates
        mid1 = Vector((imgx1 + imgx2) / 2, (imgy1 + imgy2) / 2)
        mid2 = Vector((imgx2 + imgx3) / 2, (imgy2 + imgy3) / 2)
        mid3 = Vector((imgx3 + imgx4) / 2, (imgy3 + imgy4) / 2)
        mid4 = Vector((imgx4 + imgx1) / 2, (imgy4 + imgy1) / 2)

        # Compute line of anisotropy
        diag1 = mid1 - mid3
        diag2 = mid2 - mid4

        if diag1.norm() > diag2.norm():
            Ldiag = diag1
            Sdiag = diag2
            midLdiag1 = mid1
            midLdiag2 = mid3
        else:
            Ldiag = diag2
            Sdiag = diag1
            midLdiag1 = mid2
            midLdiag2 = mid4

        # Compute the level of MipMaps to be used
        nMipmaps = 9
        n = nMipmaps + math.log(1 / Ldiag.norm(), 2)
        # level = n
        level = 3.2
        level_up = int(math.ceil(level))
        level_down = int(math.floor(level))
        # mip_up = cv2.imread(file_name + "_" + str(2**level_up) + ext)
        # mip_down = cv2.imread(file_name + "_" + str(2 ** level_down) + ext)

        #pick the mipmaps to use
        if (2**level_up == 2):
            mip_up = img2
            mip_down = img
        elif (2**level_up == 4):
            mip_up = img4
            mip_down = img2
        elif (2 ** level_up == 8):
            mip_up = img8
            mip_down = img4
        elif (2**level_up == 16):
            mip_up = img16
            mip_down = img8
        elif (2**level_up == 32):
            mip_up = img32
            mip_down = img16
        elif (2**level_up == 64):
            mip_up = img64
            mip_down = img32
        elif (2**level_up == 128):
            mip_up = img128
            mip_down = img64
        elif (2**level_up == 256):
            mip_up = img256
            mip_down = img128
        elif (2**level_up == 512):
            mip_up = img512
            mip_down = img256

        sample = Ldiag.norm() / Sdiag.norm()
        nsample = int(math.ceil(sample))

        # make nsample points on Ldiag
        # Interpolated Pixel center on the level 0 Mipmap, i.e., original texture

        # tex_pix = midLdiag1 + midLdiag2
        # tex_pix = Vector(tex_pix.x / 2, tex_pix.y / 2)

        k = Ldiag.norm() / (nsample + 1)
        tex_pix = midLdiag1 + k * (midLdiag2 - midLdiag1)
        # Compute Pixel color on the higher Mipmap level
        level_dim = (2 ** level_up)
        int_map_x = int(round(tex_pix.x / level_dim))
        int_map_y = int(round(tex_pix.y / level_dim))
        if int_map_x < 0: int_map_x = 0
        if int_map_x > 512/level_dim - 1: int_map_x = 512/level_dim-1
        if int_map_y < 0: int_map_y = 0
        if int_map_y > 512/level_dim - 1: int_map_y = 512/level_dim-1

        color_map1 = Color(mip_up[int_map_x, int_map_y, 0], mip_up[int_map_x, int_map_y, 1],
                          mip_up[int_map_x, int_map_y, 2])

        tex_pix = midLdiag1 + 2*k * (midLdiag2 - midLdiag1)
        # Compute Pixel color on the higher Mipmap level
        level_dim = (2 ** level_up)
        int_map_x = int(round(tex_pix.x / level_dim))
        int_map_y = int(round(tex_pix.y / level_dim))
        if int_map_x < 0: int_map_x = 0
        if int_map_x > 512 / level_dim - 1: int_map_x = 512 / level_dim - 1
        if int_map_y < 0: int_map_y = 0
        if int_map_y > 512 / level_dim - 1: int_map_y = 512 / level_dim - 1

        color_map2 = Color(mip_up[int_map_x, int_map_y, 0], mip_up[int_map_x, int_map_y, 1],
                           mip_up[int_map_x, int_map_y, 2])

        #interpolated color on higher mipmap
        color_map = color_map1*0.5 + color_map2*0.5

        # Compute Pixel color on the lower Mipmap level
        tex_pix = midLdiag1 + k * (midLdiag2 - midLdiag1)
        level_dim = (2 ** level_down)
        int_map_x = int(round(tex_pix.x / level_dim))
        int_map_y = int(round(tex_pix.y / level_dim))
        if int_map_x < 0: int_map_x = 0
        if int_map_x > 512/level_dim - 1: int_map_x = 512/level_dim - 1
        if int_map_y < 0: int_map_y = 0
        if int_map_y > 512/level_dim - 1: int_map_y = 512/level_dim - 1

        color_mip1 = Color(mip_down[int_map_x, int_map_y, 0], mip_down[int_map_x, int_map_y, 1],
                          mip_down[int_map_x, int_map_y, 2])

        tex_pix = midLdiag1 + 2*k * (midLdiag2 - midLdiag1)
        level_dim = (2 ** level_down)
        int_map_x = int(round(tex_pix.x / level_dim))
        int_map_y = int(round(tex_pix.y / level_dim))
        if int_map_x < 0: int_map_x = 0
        if int_map_x > 512 / level_dim - 1: int_map_x = 512 / level_dim - 1
        if int_map_y < 0: int_map_y = 0
        if int_map_y > 512 / level_dim - 1: int_map_y = 512 / level_dim - 1

        color_mip2 = Color(mip_down[int_map_x, int_map_y, 0], mip_down[int_map_x, int_map_y, 1],
                          mip_down[int_map_x, int_map_y, 2])

        # interpolated color on lower mipmap
        color_mip = color_mip1 * 0.5 + color_mip2 * 0.5

        # Tri-linear interpolation for calculation of pixel's final color value
        # color += (color_map*0.5 + color_mip*0.5)
        color += (level_up-level)*color_map + (level-level_down)*color_mip
        # '''

        '''
        #enable this code for
        # Nearest Texel
        int_imgx = int(round(imgx))
        int_imgy = int(round(imgy))
        if int_imgx == 512:
            int_imgx = 511
        if int_imgy == 512:
            int_imgy = 511
        color += Color(img[int_imgx, int_imgy, 0], img[int_imgx, int_imgy, 1], img[int_imgx, int_imgy, 2])
        '''

        '''
        #enable this code for
        # Bilinearly Interpolated Texel
        ceilx = int(math.ceil(imgx))
        ceily = int(math.ceil(imgy))
        floorx = int(math.floor(imgx))
        floory = int(math.floor(imgy))
        if ceilx >= 512:
            ceilx = 511
        if ceily >= 512:
            ceily = 511
        if floorx >= 512:
            floorx = 511
        if floory >= 512:
            floory = 511
        interpolate_x1 = (ceilx - imgx) * (img[ceilx, ceily]) + (imgx - floorx) * (img[floorx, ceily])
        interpolate_x2 = (ceilx - imgx) * (img[ceilx, floory]) + (imgx - floorx) * (img[floorx, floory])
        interpolate_y  = (ceily - imgy) * interpolate_x1 + (imgy - floory) * interpolate_x2
        color += Color(interpolate_y[0], interpolate_y[1], interpolate_y[2])
        # print color
        '''

        return color

    def _get_intersection(self, ray):
        """
        If ray intersects any of `self.objects`, return `obj, dist` (the object
        itself, and the distance to it). Otherwise, return `None`.
        """

        intersection = None
        for obj in self.objects:
            dist = obj.intersects(ray)
            if dist is not None and \
                    (intersection is None or dist < intersection[1]):
                intersection = obj, dist

        return intersection


class Vector:
    """
	A generic 3-element vector. All of the methods should be self-explanatory.
	"""

    def __init__(self, x=0, y=0, z=0):
        self.x = x
        self.y = y
        self.z = z

    def norm(self):
        return math.sqrt(sum(num * num for num in self))

    def normalize(self):
        return Vector(self.x / self.norm(), self.y / self.norm(), self.z / self.norm())

    def reflect(self, other):
        other = other.normalize()
        return self - 2 * (self * other) * other

    def __str__(self):
        return "Vector({}, {}, {})".format(*self)

    def __add__(self, other):
        return Vector(self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other):
        return Vector(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, other):
        if isinstance(other, Vector):
            return self.x * other.x + self.y * other.y + self.z * other.z;
        else:
            return Vector(self.x * other, self.y * other, self.z * other)

    def __rmul__(self, other):
        return self.__mul__(other)

    def __truediv__(self, other):
        return Vector(self.x / other, self.y / other, self.z / other)

    def __pow__(self, exp):
        if exp != 2:
            raise ValueError("Exponent can only be two")
        else:
            return self * self

    def __iter__(self):
        yield self.x
        yield self.y
        yield self.z


Point = Vector
Color = Vector


class Plane:
    def __init__(self, origin, normal, material):
        self.origin = origin
        self.normal = normal
        self.material = material

    def intersects(self, ray):
        """
        If `ray` intersects sphere, return the distance at which it does;
        otherwise, `None`.
        """
        theta = 45
        H = 512
        W = 512
        A = self.origin
        B = Point(W, A.y, A.z)
        C = Point(B.x, (int)(H * math.sin(theta * math.pi / 180)), (int)(H * math.cos(math.pi * theta / 180)))
        D = Point(A.x, (int)(H * math.sin(theta * math.pi / 180)), (int)(H * math.cos(math.pi * theta / 180)))
        vec3 = ray.direction * self.normal
        if vec3 != 0:
            vec1 = self.origin - ray.origin
            vec2 = vec1 * self.normal
            dist = vec2 / vec3
            if dist > 0:
                point_on_plane = ray.origin + dist * ray.direction
                if A.x <= point_on_plane.x <= B.x and A.y <= point_on_plane.y <= D.y and B.z <= point_on_plane.z <= C.z:
                    #print A, B, C, D, point_on_plane
                    return dist

    def surface_norm(self, pt):
        """
        Return the surface normal to the sphere at `pt`.
        """

        return self.normal.normalize()


class Material:
    def __init__(self, color, specular=0.5, lambert=1, ambient=0.6):
        self.color = color
        self.specular = specular
        self.lambert = lambert
        self.ambient = ambient


class Ray:
    """
    A mathematical ray.
    """

    def __init__(self, origin, direction):
        self.origin = origin
        self.direction = direction.normalize()

    def point_at_dist(self, dist):
        return self.origin + self.direction * dist


def pixels_to_ppm(pixels):
    """
    Convert `pixels`, a 2D array of `Color`s, into a PPM P3 string.
    """

    header = "P3 {} {} 255\n".format(len(pixels[0]), len(pixels))
    img_data_rows = []
    for row in pixels:
        pixel_strs = [
            " ".join([str(int(color)) for color in pixel]) for pixel in row]
        img_data_rows.append(" ".join(pixel_strs))
    return header + "\n".join(img_data_rows)


if __name__ == "__main__":
    objects = [
        Plane(Point(0, 0, 0), Point(0, -1, 1), Material(Color(0, 0, 0)))
    ]
    lights = [Point(200, -100, 0), Point(600, 200, -200)]
    camera = Point(256, 256, -100)
    scene = Scene(camera, objects, lights, 512, 512)
    pixels = scene.render()
    with open("image.ppm", "w") as img_file:
        img_file.write(pixels_to_ppm(pixels))
