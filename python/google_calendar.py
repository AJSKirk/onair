from __future__ import print_function
import pickle
import os.path
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request
from datetime import datetime
import click
import requests
import json

# If modifying these scopes, delete the file token.pickle.
SCOPES = ['https://www.googleapis.com/auth/calendar.readonly']
API_URL = "https://www.googleapis.com/calendar/v3/freeBusy"
SECRETS_FILE = "credentials.json"
API_KEY = 'PLS_NO_STEAL'


def get_creds():
    """Authorisation boilerplate from Google's quickstart template"""
    creds = None
    # The file token.pickle stores the user's access and refresh tokens, and is
    # created automatically when the authorization flow completes for the first
    # time.
    if os.path.exists('token.pickle'):
        with open('token.pickle', 'rb') as token:
            creds = pickle.load(token)
    # If there are no (valid) credentials available, let the user log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(
                SECRETS_FILE, SCOPES)
            creds = flow.run_local_server(port=0)
        # Save the credentials for the next run
        with open('token.pickle', 'wb') as token:
            pickle.dump(creds, token)
    return creds


def get_busies(creds, time_min: datetime, time_max: datetime, email: str, time_zone: str = "UTC"):
    service = build('calendar', 'v3', credentials=creds)

    headers = {"Authorization": "Bearer {}".format(creds.token),
              "Accept": "application/json",
              "Content-Type": "application/json"}

    # I think ISOFormat works, else its YYYY-MM-DDTHH:MM:SS.ppZ
    body = {"timeMin": time_min.isoformat() + 'Z', "timeMax": time_max.isoformat() + 'Z', "timeZone": time_zone,
            "items": [{"id": email}]}

    r = requests.post(API_URL, params={"key": API_KEY}, headers=headers, data=json.dumps(body))

    return json.loads(r.text)['calendars'][email]['busy']


@click.command()
@click.argument('email')
def main(email):
    """Retrieves today's busy times for user email supplied as command line argument"""
    creds = get_creds()
    start = datetime.now().replace(hour=0, minute=0, second=0)
    end = datetime.now().replace(hour=23, minute=59, second=59)
    busies = get_busies(creds, start, end, email)
    print(busies)


if __name__ == '__main__':
    main()
