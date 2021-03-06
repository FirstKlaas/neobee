"""
Simple dumper of the neobee board messages
to a human readable form.

Copyright FirstKlaas 2021
"""
import asyncio as aio
from functools import reduce
import logging
import os

import aio_mqtt
from dotenv import load_dotenv
from tortoise import Tortoise

from .models.board import BoardModel
from .models.measure import MeasurementModel

from .influx import HiveData
from .orm import init_models

logger = logging.getLogger(__name__)


async def mqtt_connect_forever(client: aio_mqtt.Client) -> None:
    while True:
        try:
            logger.debug("Connecting to host %s", os.getenv("MQTT_HOST", "broker.hivemq.com"))
            connect_result = await client.connect(
                os.getenv("MQTT_HOST", "broker.hivemq.com"),
                port=int(os.getenv("MQTT_PORT", "1883")),
                client_id="neobee_relay",
                username=os.getenv("MQTT_LOGIN"),
                password=os.getenv("MQTT_PASSWORD"),
            )
            logger.debug("Connected")
            await client.subscribe(("/neobee/#", aio_mqtt.QOSLevel.QOS_1))
            logger.debug("Subscribed")
            await connect_result.disconnect_reason
        except Exception as e:
            logger.error("MQTT Connection lost.", exc_info=e)


async def board_disconnected(payload):
    logger.info("Board %s disconnected", bytearray(payload).decode())


async def create_or_update_board(payload):
    board_defaults = {}
    mac = ":".join([hex(b)[2:4].upper() for b in payload[:6]])
    logger.info("Board %s connected", mac)
    board_defaults["ip"] = ".".join([str(b).zfill(3) for b in payload[6:10]])
    try:
        index = payload[13:33].index(0)
    except ValueError:
        index = 20

    board_defaults["firmware"] = f"{payload[10]}.{payload[11]}.{payload[12]}"
    board_defaults["name"] = bytearray(payload[13 : 13 + index]).decode()
    logger.info("Data: %r", board_defaults)
    await BoardModel.update_or_create(board_defaults, mac=mac)


def f100(arr):
    """
    Decodes a four bytes encoded signed float
    value.
    The highest bit of the first byte indicates
    wether the value is negative (1) or not (0).
    """
    NEG_FLAG = arr[0] & 0b10000000
    if NEG_FLAG:
        # Clear the negative flag
        value = ((arr[0] & 0b01111111) << 24) | (arr[1] << 16) | (arr[2] << 8) | arr[3]
        return (-1 * value) / 100.0
    else:
        return ((arr[0] << 24) | (arr[1] << 16) | (arr[2] << 8) | arr[3]) / 100


async def new_hive_data(hive_data: HiveData, payload):
    mac = ":".join([hex(b)[2:4].upper() for b in payload[:6]])
    weight = f100(payload[6:10])
    temperature_inside = f100(payload[10:14])
    temperature_outside = f100(payload[14:18])
    logger.debug("Data: %s / %s / %s / %s", mac, weight, temperature_inside, temperature_outside)
    await hive_data.write_point(mac, weight, temperature_inside, temperature_outside)

    measurement_defaults = {
        "weight": weight,
        "temperature_inside": temperature_inside,
        "temperature_outside": temperature_outside,
    }
    m = await MeasurementModel.update_or_create(measurement_defaults, mac=mac)
    logger.info(m)


async def handle_message(client: aio_mqtt.Client, hive_data: HiveData, loop):
    async for message in client.delivered_messages("#"):
        while True:
            try:
                logger.debug(message.topic_name)
                if message.topic_name == "/neobee/hive/connect":
                    loop.create_task(create_or_update_board(message.payload))
                elif message.topic_name == "/neobee/hive/disconnect":
                    loop.create_task(board_disconnected(message.payload))
                elif message.topic_name == "/neobee/hive/rawdata":
                    loop.create_task(new_hive_data(hive_data, message.payload))

            except aio_mqtt.ConnectionClosedError as e:
                await client.wait_for_connect()
                continue

            except Exception as e:
                logger.error("Unable to handle messages.", exc_info=e)

            break


async def close(client: aio_mqtt.Client, tasks) -> None:
    for task in tasks:
        if task.done():
            continue
        task.cancel()
        try:
            await task
        except aio.CancelledError:
            pass
    if client.is_connected():
        await client.disconnect()


def run():
    logging.basicConfig(level="DEBUG")
    load_dotenv(verbose=False)

    logger.info(os.getenv("MQTT_HOST"))
    hive_data = HiveData(
        os.getenv("INFLUX_HOST"),
        os.getenv("INFLUX_PORT"),
        os.getenv("INFLUX_BUCKET"),
        os.getenv("INFLUX_TOKEN"),
        os.getenv("INFLUX_ORG"),
    )

    loop = aio.new_event_loop()
    client = aio_mqtt.Client(loop=loop)
    loop.run_until_complete(init_models())
    tasks = [
        loop.create_task(mqtt_connect_forever(client)),
        loop.create_task(handle_message(client, hive_data, loop)),
    ]
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        pass
    finally:
        loop.run_until_complete(close(client, tasks))
        loop.close()


if __name__ == "__main__":
    run()
