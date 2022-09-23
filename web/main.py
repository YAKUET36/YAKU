from pymongo import MongoClient 
from flask import Flask, render_template
import os

MONGO_URI = os.getenv('mongo_user')#importing our secret key
     
client = MongoClient(MONGO_URI) #here we import the mongo client
 
mydata = list(client.YakuNodos['Nodos'].find()) #searching for the info
lenght = len(mydata) - 1 #selecting the last one
last_callback = mydata[lenght] #creating the callback

app = Flask(__name__) 
 
@app.route("/", methods=["GET", "POST"]) 
def hello_world(): 
    global last_callback

    return render_template('home.html', soil_m=last_callback['Soil Moisture'], 
        ldr=last_callback['LDR'], press=last_callback['Pressure'], 
        hum=last_callback['Humidity'], temp=last_callback['Temperature'], 
        relay = last_callback['Relay'], wind=last_callback['Wind']) #sending the parameters to the page
        #height=last_callback['Height']

if __name__ == "__main__":
    print(last_callback)
    app.run(debug=True)
