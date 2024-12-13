from skimage import io
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


image = io.imread('captures_LI-S5K2G1_0.png')
plt.imshow(image)
#plt.show()