import matplotlib.cm
import io

from flask import Flask, send_file
from zaczynski import MandelbrotSet, Viewport, paint, denormalize
from PIL import Image

app = Flask(__name__)

def MandelbrotSetImage(palette, center_real, center_imag, height, dim, iterations):
  image = Image.new(mode="RGB", size=(dim, dim))
  paint(
    MandelbrotSet(max_iterations=iterations, escape_radius=1000),
    Viewport(image, center=complex(center_real, center_imag), width=height),
    palette,
    smooth=True
  )
  return image

@app.route('/mandelbrot/<colormap>/<float(signed=True):real>:<float(signed=True):imag>:<float:height>:<int:dim>:<int:iter>', methods=['GET'])
def waf(colormap, real, imag, height, dim, iter):
  try:
    colormap = matplotlib.cm.get_cmap(colormap).colors
    palette = denormalize(colormap)
  except Exception as e:
    return str(e), 400

  image = MandelbrotSetImage(palette, real, imag, height, dim, iter)
  memoryBuffer = io.BytesIO()
  image.save(memoryBuffer, "PNG")
  memoryBuffer.seek(0)
  return send_file(memoryBuffer, mimetype="image/png")