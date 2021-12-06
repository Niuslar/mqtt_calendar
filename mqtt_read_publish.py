from __future__ import print_function
import datetime
import time
import os.path
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
import paho.mqtt.client as mqtt
import calendar

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc==0:
        print("connected OK Returned code=",rc)

    else:
        print("Bad connection Returned code=",rc)

    client.subscribe("/events")
    client.subscribe("/month_begin")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

def on_publish(client, obj, mid):
    print("Event Sent")

def on_subscribe(client, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

#MQTT Configuration
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.on_publish = on_publish
client.on_subscribe = on_subscribe
client.username_pw_set(username="username",password="password")
client.connect("host", port, 60)

# If modifying these scopes, delete the file token.json.
SCOPES = ['https://www.googleapis.com/auth/calendar']

def get_events(service, calendar_id):
    now = datetime.datetime.utcnow().isoformat() + 'Z' # 'Z' indicates UTC time
    events_result = service.events().list(calendarId = calendar_id, 
                                                    timeMin = now,
                                                    singleEvents = True,
                                                    orderBy = 'startTime').execute()
    
    return events_result.get('items', [])

def send_event(event_day, calendar_color_id, first_of_month):
    #Get day, month, year from date
    current_date = datetime.date.today().day
    client.publish("/events", f"{event_day},{calendar_color_id},{first_of_month},{current_date}", 0)
    
def main():
    creds = None
    # The file token.json stores the user's access and refresh tokens, and is
    # created automatically when the authorization flow completes for the first
    # time.
    if os.path.exists('token.json'):
        creds = Credentials.from_authorized_user_file('token.json', SCOPES)
    # If there are no (valid) credentials available, let the user log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(
                'credentials.json', SCOPES)
            creds = flow.run_local_server(port=0)
        # Save the credentials for the next run
        with open('token.json', 'w') as token:
            token.write(creds.to_json())

    service = build('calendar', 'v3', credentials=creds)

    # Call the Calendar API

    #Add to this list the Calendar ID of the calendars you don't want to show (example below)
    #hidden_calendars = ['es.cl#holiday@group.v.calendar.google.com']
    hidden_calendars = []
    page_token = None
    while True:
        #Get the current year, month and days in a month 
        current_year = datetime.date.today().year
        current_month = datetime.date.today().month
        days_in_month = calendar.monthrange(datetime.date.today().year, datetime.date.today().month)[1]
        first_of_month = calendar.monthrange(current_year,current_month)[0]
        # Get the data from google calendar
        calendar_list = service.calendarList().list(pageToken=page_token).execute()
        # This list will hold the days where there are events
        events_dates = []
        # Loop through the different calendars and the events on each calendar
        for calendar_list_entry in calendar_list['items']:
            if calendar_list_entry['id'] not in hidden_calendars:
                events = get_events(service, calendar_list_entry['id'])
                for event in events:
                    # Get the relevant data from the event
                    event_date_full = event['start'].get('dateTime', event['start'].get('date'))
                    event_date = event_date_full[0:10]
                    event_year = datetime.datetime.strptime(event_date,"%Y-%m-%d").year
                    event_month = datetime.datetime.strptime(event_date,"%Y-%m-%d").month
                    event_day = event_day = datetime.datetime.strptime(event_date,"%Y-%m-%d").day
                    # Send event only if the month and year matches
                    if current_month == event_month and current_year == event_year: 
                        events_dates.append(event_day)
                        send_event(event_day, calendar_list_entry['colorId'],first_of_month)
        # To update the ESP32 when an event has been deleted, we need to send a list 
        # with all the days of the month that do not have an event
        for day in range(days_in_month):
            if day+1 not in events_dates:
                print(day)
                # 25 is the made up colour id that turns the LED off
                send_event(f"{day+1}", 25, first_of_month)
        page_token = calendar_list.get('nextPageToken')
        # Google has a limit of how often you can get the data, so we wait 3 seconds before repeating the loop
        time.sleep(3)
        #page_token = None
if __name__ == '__main__':
    main()
