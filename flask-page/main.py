from pymongo import MongoClient 
from flask import Flask, render_template
from account import mongo_user #importing our secret key
     
client = MongoClient(mongo_user) #here we import the mongo client
 
mydata = list(client.YakuNodos['Nodos'].find()) #searching for the info
lenght = len(mydata) - 1 #selecting the last one
last_callback = mydata[lenght] #creating the callback

app = Flask(__name__) 
 
@app.route("/", methods=["GET", "POST"]) 
def hello_world(): 
    global last_callback

    return render_template('home.html', hum=last_callback['Humidity'],
     ldr=last_callback['LDR'], soil_m=last_callback['Soil Moisture'],
      temp=last_callback['Temperature'], press=last_callback['Pressure']) #sending the parameters to the page

if __name__ == "__main__":
    print(last_callback)
    app.run()
    print(last_callback)
