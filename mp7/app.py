from flask import Flask, render_template, send_file, request
import os

app = Flask(__name__)
# Route for "/" for a web-based interface to this micro-service:

@app.route('/')
def index():
  return render_template("index.html")


# Extract a hidden "uiuc" GIF from a PNG image:
@app.route('/extract', methods=["POST"])
def extract_hidden_gif():
  png_file = request.files['png']
  png_file_name = png_file.filename
  png_file.save(png_file_name)
  file_list= os.listdir("./temp")
  file_num = 0;
  for files in file_list:
    if(files[-4:] == ".gif"):
      file_num = file_num + 1;
  gif_index = file_num

  gif_file_path = "temp/gif" + str(gif_index) + ".gif"
  command = "./png-extractGIF" + " " + png_file_name + " " + gif_file_path
  status_code = os.system(command)
  os.remove(png_file_name)
  if (status_code == 40704):
    return "Unprocessable Content", 415
  elif(status_code == 512):
    return "Unsupported Media Type", 422
  else:
    return send_file(gif_file_path), 200
# Get the nth saved "uiuc" GIF:
@app.route('/extract/<int:image_num>', methods=['GET'])
def extract_image(image_num):
  # ...
  file_list= os.listdir("./temp")
  file_num = 0;
  for files in file_list:
    if(files[-4:] == ".gif"):
      file_num = file_num + 1;
  if (image_num >= file_num):
    return "Not Found", 404
  gif_file_path  = "temp/gif" + str(image_num) + ".gif"
  return send_file(gif_file_path), 200
