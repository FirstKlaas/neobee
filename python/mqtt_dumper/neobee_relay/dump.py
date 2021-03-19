"""
Simple dumper of the neobee board messages
to a human readable form.

Copyright FirstKlaas 2021
"""
import asyncio as aio
from functools import reduce
import logging
import os
from datetime import datetime

import aio_mqtt
from dotenv import load_dotenv

logger = logging.getLogger(__name__)


async def mqtt_connect_forever(client: aio_mqtt.Client) -> None:
    while True:
        try:
            connect_result = await client.connect("broker.hivemq.com")
            logger.debug("Connected")
            await client.subscribe(("/neobee/#", aio_mqtt.QOSLevel.QOS_1))
            logger.debug("Subscribed")
            await connect_result.disconnect_reason
        except Exception as e:
            logger.error("MQTT Connection lost.", exc_info=e)


async def board_disconnected(payload):
    logger.info("Board %s disconnected", bytearray(payload).decode())


async def create_or_update_board(payload):
    mac = ":".join([hex(b)[2:4].upper() for b in payload[:6]])
    logger.info("Board %s connected", mac)


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


async def new_hive_data(payload):
    mac = ":".join([hex(b)[2:4].upper() for b in payload[:6]])
    weight = f100(payload[6:10])
    temperature_inside = f100(payload[10:14])
    temperature_outside = f100(payload[14:18])
    logger.info("-" * 60)
    logger.info(f"Timestampe          : {datetime.now()}")
    logger.info(f"MAC                 : {mac}")
    logger.info(f"Weight              : {weight}")
    logger.info(f"Temperature Inside  : {temperature_inside}")
    logger.info(f"Temperature Outside : {temperature_outside}")


async def handle_message(client: aio_mqtt.Client, loop):
    async for message in client.delivered_messages("#"):
        while True:
            try:
                logger.info(f"Topic               : {message.topic_name}")
                if message.topic_name == "/neobee/hive/connect":
                    loop.create_task(create_or_update_board(message.payload))
                elif message.topic_name == "/neobee/hive/disconnect":
                    loop.create_task(board_disconnected(message.payload))
                elif message.topic_name == "/neobee/hive/rawdata":
                    loop.create_task(new_hive_data(message.payload))

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

    loop = aio.new_event_loop()
    client = aio_mqtt.Client(loop=loop)
    tasks = [
        loop.create_task(mqtt_connect_forever(client)),
        loop.create_task(handle_message(client, loop)),
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
