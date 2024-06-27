from ants import utils
import time
import numpy as np
import cv2

np.set_printoptions(suppress=True)

fixed_image = cv2.imread("f.jpg", cv2.IMREAD_GRAYSCALE)
moving_image = cv2.imread("f.jpg", cv2.IMREAD_GRAYSCALE)

fixed_image = fixed_image.reshape((1344, 524), order="F")
moving_image = moving_image.reshape((1344, 524), order="F")

cudaRegistration = utils.get_lib_fn("cudaRegistration")

for i in range(2):
    print("registration start!")
    start = time.time()
    Gc = cudaRegistration(fixed_image, moving_image)
    end = time.time()
    print("registration finsih! time cost", end - start)

fixed_image = cv2.imread("f.jpg", cv2.IMREAD_GRAYSCALE)
fixed_image = fixed_image.astype(np.int32)

ants_img = Gc.reshape((524, 1344), order="F")

diff_img = cv2.absdiff(fixed_image, ants_img)

cv2.imwrite("diff4.jpg", diff_img)
