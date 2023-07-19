import json
from flask import Flask, jsonify, send_file, render_template, request
import requests
import os
import io
import boto3
import base64
import dotenv
import math

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/all')
def all():
    return render_template('all.html')

@app.route('/moveUp', methods=["POST"])
def moveUp():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    data["imag"] = data["imag"] + data["height"] / 4
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(data, file_object)
    
    return jsonify(data), 200

@app.route('/moveDown', methods=["POST"])
def moveDown():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    data["imag"] = data["imag"] - data["height"] / 4
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(data, file_object)
    return jsonify(data), 200

@app.route('/moveLeft', methods=["POST"])
def moveLeft():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    data["real"] = data["real"] - data["height"] / 4
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(data, file_object)
    return jsonify(data), 200

@app.route('/moveRight', methods=["POST"])
def moveRight():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    data["real"] = data["real"] + data["height"] / 4
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(data, file_object)
    return jsonify(data), 200

@app.route('/zoomIn', methods=["POST"])
def zoomIn():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    data["height"] = data["height"] * (1/1.4)
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(data, file_object)
    return jsonify(data), 200

@app.route('/zoomOut', methods=["POST"])
def zoomOut():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    data["height"] = data["height"] * (1.4)
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(data, file_object)
    return jsonify(data), 200

@app.route('/smallerImage', methods=["POST"])
def smallerImage():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 

    new_data = {'dim' : round(data["dim"] * (1/1.25))}
    for items in data:
        if items != 'dim':
            new_data[items] = data[items]

    #data["dim"] = (data["dim"] * (1/1.25))

    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(new_data, file_object)
    return jsonify(new_data), 200

@app.route('/largerImage', methods=["POST"])
def largerImage():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 

    new_data = {'dim' : round(data["dim"] * (1.25))}
    for items in data:
        if items != 'dim':
            new_data[items] = data[items]
    
    #data["dim"] = (data["dim"] * (1.25))
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(new_data, file_object)
    return jsonify(new_data), 200

@app.route('/moreIterations', methods=["POST"])
def moreIterations():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 

    new_data = {'iter' : round(data["iter"] * 2)}
    for items in data:
        if items != 'iter':
            new_data[items] = data[items]

    #data["iter"] = int(data["iter"] * (2))
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(new_data, file_object)
    return jsonify(new_data), 200

@app.route('/lessIterations', methods=["POST"])
def lessIterations():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    new_data = {'iter' : round(data["iter"] * (1/2))}
    for items in data:
        if items != 'iter':
            new_data[items] = data[items]
    #data["iter"] = int(data["iter"] * (1/2))
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(new_data, file_object)
    return jsonify(new_data), 200

@app.route('/changeColorMap', methods=["POST"])
def changeColorMap():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    changed_color = request.get_json()["colormap"]
    data["colormap"] = changed_color
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(data, file_object)
    return jsonify(data), 200

@app.route('/mandelbrot', methods=["GET"])
def getmandelbrot():
    s3_client = boto3.client('s3', endpoint_url = "http://127.0.0.1:9000", aws_access_key_id = "ROOTNAME",  aws_secret_access_key = "CHANGEME123")
    
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        current_state = json.load(file_object)
    png_list = s3_client.list_objects_v2(Bucket='mandelbrot')
   
    if (png_list['KeyCount'] == 0):
        server_url = os.getenv('MANDELBROT_MICROSERVICE_URL')
        
        request_png = server_url + "/mandelbrot/" + current_state["colormap"] + "/" + str(current_state["real"]) + ":" + str(current_state["imag"]) + ":" + str(current_state["height"]) + ":" + str(round(current_state["dim"])) + ":" + str(round(current_state["iter"]))

        with open("request.txt", 'w') as f:
            f.write(request_png)
        maldelbrot_png = requests.get(request_png)
        file_name = current_state["colormap"] + ":" + str(current_state["real"]) + ":" + str(current_state["imag"]) + ":" + str(current_state["height"]) + ":" + str(round(current_state["dim"])) + ":" + str(round(current_state["iter"]))+ ".png"
        temp = "temp.png"
        with open(temp, 'wb') as f:
            f.write(maldelbrot_png.content)

        with open(temp, "rb") as f:
            s3_client.upload_fileobj(f, "mandelbrot", file_name)
        return send_file(temp, mimetype="image/png"), 200
   
    for png_files in png_list['Contents']:
        png_name = png_files['Key'][:-4]
        split_by_colon = png_name.split(":")
        colormap = split_by_colon[0]
        #split_by_colon = split_by_slash[1].split(":")
        real = float(split_by_colon[1])
        imag = float(split_by_colon[2])
        height = float(split_by_colon[3])
        dim = float(split_by_colon[4])
        iter = float(split_by_colon[5])
        if (current_state["colormap"] == colormap and math.isclose(current_state["real"], real) and math.isclose(current_state["imag"], imag) and
            math.isclose(current_state["height"], height) and math.isclose(current_state["dim"], dim) and math.isclose(current_state["iter"], iter)):
            object_name = png_files['Key']
            temp = "temp.png"
            with open(temp, 'wb') as f:
                s3_client.download_fileobj("mandelbrot", object_name, f)
            return send_file(temp, mimetype="image/png"), 200
        
    server_url = os.getenv('MANDELBROT_MICROSERVICE_URL')

    request_png = server_url + "/mandelbrot/" + current_state["colormap"] + "/" + str(current_state["real"]) + ":" + str(current_state["imag"]) + ":" + str(current_state["height"]) + ":" + str(round(current_state["dim"])) + ":" + str(round(current_state["iter"]))

    with open("request.txt", 'w') as f:
        f.write(request_png)

    maldelbrot_png = requests.get(request_png)
    file_name = current_state["colormap"] + ":" + str(current_state["real"]) + ":" + str(current_state["imag"]) + ":" + str(current_state["height"]) + ":" + str(round(current_state["dim"])) + ":" + str(round(current_state["iter"])) + ".png"
    temp = "temp.png"
    with open(temp, 'wb') as f:
        f.write(maldelbrot_png.content)

    with open(temp, "rb") as f:
        s3_client.upload_fileobj(f, "mandelbrot", file_name)
    return send_file(temp, mimetype="image/png"), 200

@app.route('/storage', methods=["GET"])
def storage():
    toReturn = [];
    s3_client = boto3.client('s3', endpoint_url = "http://127.0.0.1:9000", aws_access_key_id = "ROOTNAME",  aws_secret_access_key = "CHANGEME123")
    png_list = s3_client.list_objects_v2(Bucket='mandelbrot')
    for png_files in png_list['Contents']:
        png_name = png_files['Key']
        temp = "temp.png"
        with open(temp, 'wb') as f:
            s3_client.download_fileobj("mandelbrot", png_name, f)
        b64_string = ""
        with open(temp, "rb") as img_file:
            b64_string = base64.b64encode(img_file.read())
        to_append = {}
        to_append["key"] = png_name[-4:]
        to_append["image"] = "data:image/png;base64," + str(b64_string)
        toReturn.append(to_append)

    return jsonify(toReturn), 200

@app.route('/resetTo', methods=["POST"])
def resetTo():
    new_json = request.get_json()
    with open('dynamic_mandelbrot.json', 'w') as file_object:
        json.dump(new_json, file_object)
    
    return "OK", 200

@app.route('/getState', methods=["GET"])
def getState():
    data = ""
    with open('dynamic_mandelbrot.json', 'r') as file_object:  
        data = json.load(file_object) 
    data['dim'] = round(data['dim'])
    data['iter'] = round(data['iter'])
    return jsonify(data), 200

@app.route('/clearCache', methods=["GET"])
def clearCache():
    s3_client = boto3.client('s3', endpoint_url = "http://127.0.0.1:9000", aws_access_key_id = "ROOTNAME",  aws_secret_access_key = "CHANGEME123")
    png_list = s3_client.list_objects_v2(Bucket='mandelbrot')
    if (png_list['KeyCount'] > 0):
        for png_files in png_list['Contents']:
            s3_client.delete_object(Bucket='mandelbrot', Key=png_files['Key'])
    
    return "OK", 200
