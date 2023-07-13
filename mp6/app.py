from flask import Flask, render_template, request, jsonify
import os
import requests
from datetime import datetime, timedelta, time
import json
import pytz
app = Flask(__name__)


def pull_weather_data(current_time, file_name):
    r_weather = requests.get("https://api.weather.gov/gridpoints/ILX/95,71/forecast/hourly")
 

    weather_json = r_weather.json()["properties"]["periods"]

    expire_time =  datetime.strptime(str(current_time.replace(hour = 17, minute= 0, second=0)).split(".")[0],  "%Y-%m-%d %H:%M:%S")
    if ((expire_time - current_time).total_seconds() < 0):
        expire_time = expire_time + timedelta(1)

    weather_cached = {} 
    weather_cached['log_time'] = weather_json[0]["startTime"][:19].replace("T"," ")
    weather_cached['expire_time'] = str(expire_time)
    weather_cached['weather_json'] = weather_json
    with open(file_name, 'w') as file_object:
        json.dump(weather_cached, file_object)
 
    data = ""
    with open(file_name, 'r') as file_object:  
         data = json.load(file_object) 
    return data

def parse_course_str(course):
    course_subject = ""
    course_number = ""
    parsed_subject = 0
    for i in range(0, len(course)):
        if (course[i] == " "):
            parsed_subject = 1
            continue
        elif (course[i].isdigit() and parsed_subject == 0):
            course_number = course_number + course[i]
            parsed_subject = 1
        elif(parsed_subject == 0):
            course_subject = course_subject + course[i].upper()
        else:
            course_number = course_number + course[i]
    return (course_subject, course_number)


def next_meet_date(current_time, course_info):
    weekday_list = [];
    week_day_convertion = {"M" : 0, "T" : 1, "W" : 2, "R" : 3, "F" : 4, "S" : 5, "U" : 6};
    for day in course_info["Days of Week"]:
       weekday_list.append(week_day_convertion[day])
        
    time_diff = float('inf');
    next_date = "";
    for meeting_days in weekday_list:
        days_ahead = meeting_days - current_time.weekday()
        if days_ahead <= 0: # Target day already happened this week
            days_ahead += 7;
        hour = int(course_info["Start Time"][0:2])
        
        minute = int(course_info["Start Time"][3:5])
        meridiem = course_info["Start Time"][-2:]
        if (meridiem == "PM" and hour != 12):
            hour = hour + 12;
        date_candidate = current_time + timedelta(days_ahead)
        date_candidate = date_candidate.replace(minute=minute, hour=hour%24, second=0);
        
        second_diff = (date_candidate - current_time).total_seconds();
        if (second_diff < time_diff):
            time_diff = second_diff;
            next_date = date_candidate;
    
    return next_date;


# Route for "/" (frontend):
@app.route('/')
def index():
  return render_template("index.html")


# Route for "/weather" (middleware):
@app.route('/weather', methods=["POST"])
def POST_weather():
  course = request.form["course"]
  server_url = os.getenv('COURSES_MICROSERVICE_URL')
  course_subject, course_number = parse_course_str(course)
  
  # ...
  r = requests.get(server_url + "/" + course_subject + "/" + course_number)
  if (r.status_code == 200):
     course_json = r.json()
     converted_tz = pytz.timezone('US/Central')
     current_time = datetime.strptime(str(datetime.now(converted_tz)).split(".")[0], "%Y-%m-%d %H:%M:%S")
     next_lec_date = next_meet_date(current_time, course_json)
     hour_difference = (next_lec_date - current_time).total_seconds() / 3600 
     if (hour_difference > 144):
        result = { "course": f"{course_subject} {course_number}" }
        result["nextCourseMeeting"] = str(next_lec_date).split('.')[0]
        result["forecastTime"] = str(next_lec_date).split('.')[0]
        result["temperature"] = "forecast unavailable"
        result["shortForecast"] = "forecast unavailable"
        return jsonify(result), 200
     else:
        weather_data = get_cached_weather()
        if(len(weather_data) == 0):
            weather_data = pull_weather_data(current_time, "weather.json")
        log_time = datetime.strptime(weather_data["log_time"],  "%Y-%m-%d %H:%M:%S")
        hour_diff_from_log_start = int((next_lec_date - log_time).total_seconds() / 3600)

        data_to_look = weather_data["weather_json"][hour_diff_from_log_start]
        
        response = {}
        course_sub, course_num = parse_course_str(course)
        response["course"] = course_sub + " " + course_num
        response["nextCourseMeeting"] = str(next_lec_date).split('.')[0]
        response["forecastTime"] = data_to_look["startTime"][:19].replace("T"," ")
        response["temperature"] = data_to_look["temperature"]
        response["shortForecast"] = data_to_look["shortForecast"]
        return jsonify(response), 200
        
        
  else:
      
      return {}, 400
        



# Route for "/weatherCache" (middleware/backend):
@app.route('/weatherCache')
def get_cached_weather():
  # ...
  file_name = "weather.json"
  file_path = "./weather.json"
  converted_tz = pytz.timezone('US/Central')
  current_time = datetime.strptime(str(datetime.now(converted_tz)).split(".")[0], "%Y-%m-%d %H:%M:%S")
  if (os.path.isfile(file_path) == 0):
    return {}
  data = ""
  with open(file_name, 'r') as file_object:  
    data = json.load(file_object) 
  expire_time = datetime.strptime(data["expire_time"],  "%Y-%m-%d %H:%M:%S")
  if ((expire_time - current_time).total_seconds() < 0):
     return pull_weather_data(current_time, file_name)
  else:
     return data
  



