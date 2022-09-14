import requests
import json
from account import data, mongo_user, owm
from pymongo import MongoClient
from time import strftime
from datetime import datetime
from pyowm import OWM
from schedule import every, run_pending

def main():
    #WEATHER
    owm_object = OWM(owm) #owm object to create the weather manager with our account
    mgr = owm_object.weather_manager() #weather manager
    observation = mgr.weather_at_place('Buenos Aires, Agentina') #search for current weather in London (Buenos Aires) and get details
    w = observation.weather #stores the weather's values

    #TIMESTAMP
    ct = datetime.now() #stores the date's values
    ts = ct.timestamp() #stores actual timestamp 

    #RAINMAKER CONNECTION
    #data = account.data #references to our own librery that contains the data account.
    response = requests.post(url='https://api.rainmaker.espressif.com/v1/login', data=json.dumps(data)) #returns the tokens
    response = json.loads(response.content) #loads token's content
    headers = {"Authorization": response['accesstoken']} #defines the headers
    
    #NODE CONSULT
    node = json.loads((requests.get(
            url="https://api.rainmaker.espressif.com/v1/user/nodes/params?node_id=aYbCT4UQsbwvCEtrWCYrZg", #node id got from the app
             headers=headers).content)) #recopilates the info from the node
    
    dicc = {'Humidity': node['Humedad']['Temperature'], 'LDR': node['LDR']['Temperature'], 'Pressure' : node['Presion']['Temperature'],
            'Soil Moisture': node['Tierra']['Temperature'], 'Temperature': node['Temperatura']['Temperature'],
            'Relay' : node['Relay']['Power'], 'Wind' : str(w.wind()['speed']) + 'm/s','Clouds' : str(w.clouds) + '%',
            'Time': strftime("%d %b %Y, %H:%M") ,'Timestamp' : ts} #creates the dicc that we are sending to the database
    
    #to see the different nodes, we could use this nodes_list variable
    #nodes_list = json.loads(requests.get(url='https://api.rainmaker.espressif.com/v1/user/nodes', headers=headers).content)
    
    return dicc

def data_base():
    client = MongoClient(mongo_user)
    db = client.YakuNodos #database
    collection = db['Nodos'] #here we selectionate the collection which we are storing the data
    collection.insert_many([main()]) #inserts the data into the collection
    print(f'data sent to database: {main()}')

if __name__ == '__main__':
    #while True, and change the time on every()
    data_base()
    run_pending()
    every(3).seconds.do(data_base)