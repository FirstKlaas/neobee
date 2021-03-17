import os

from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

bucket = os.getenv("INFLUX_BUCKET")
host = os.getenv("INFLUX_HOST")
port = os.getenv("INFLUX_PORT")
org = os.getenv("INFLUX_ORG")
token = os.getenv("INFLUX_TOKEN")


class HiveData(object):
    def __init__(self, host: str, port: int, bucket: str, token: str, org: str):
        url = f"http://{host}:{port}"
        self._bucket = bucket
        self._client = InfluxDBClient(url=url, token=token, org=org)
        self._write_api = self._client.write_api(write_options=SYNCHRONOUS)
        self._query_api = self._client.query_api()

    async def write_point(self, mac: str, weight: float, temp_in: float, temp_out: float) -> None:
        p = (
            Point("hivedata")
            .tag("board", mac)
            .field("weight", weight)
            .field("temperature_inside", temp_in)
            .field("temperature_outside", temp_out)
        )
        self._write_api.write(bucket=self._bucket, record=p)
