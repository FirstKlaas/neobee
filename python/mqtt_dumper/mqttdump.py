"""
Simple dumper of the neobee board messages
to a human readable form.

Copyright FirstKlaas 2021
"""
import asyncio as aio
import logging

import aio_mqtt

from tortoise import Tortoise

from models.board import BoardModel

logger = logging.getLogger(__name__)

async def init_models():
    # Here we connect to a SQLite DB file.
    # also specify the app name of "models"
    # which contain models from "app.models"
    logger.debug("Connecting to database")
    await Tortoise.init(
        db_url='postgres://neobee:neobee@192.168.178.49:5432/neobee',
        modules={'models': ['models.board']}
    )
    # Generate the schema
    logger.debug("Generating schema")
    await Tortoise.generate_schemas()

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


async def handle_message(client: aio_mqtt.Client):
    async for message in client.delivered_messages("#"):
        while True:
            try:
                logger.debug(message.topic_name)
                if message.topic_name == "/neobee/hive/connect":
                    logger.info("New Board connected")
                    
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


def on_message(client, userdata, message):
    # print("message received ")
    # print("message topic=",message.topic)
    # print("message qos=",message.qos)
    # print("message retain flag=",message.retain)
    def shift_add(a, b):
        return a * 8 + b

    if message.topic == "/neobee/hive/disconnect":
        logger.debug("Device disconnected: ", message.payload)
    elif message.topic == "/neobee/hive/connect":
        logger.debug("Device connected:")
    elif message.topic == "/neobee/hive/rawdata":
        logger.debug("New data")
        mac = ":".join([hex(b)[2:4].upper() for b in message.payload[:6]])
        logger.debug("Mac Address: ", mac)
        from functools import reduce

        weight = reduce(lambda a, b: a * 8 + b, message.payload[6:10]) / 100
        logger.debug("Weight: ", weight)

    else:
        print("Unhandled topic ", message.topic)


if __name__ == "__main__":
    logging.basicConfig(
        level='DEBUG'
    )
    loop = aio.new_event_loop()
    client = aio_mqtt.Client(loop=loop)
    tasks = [
        loop.create_task(mqtt_connect_forever(client)),
        loop.create_task(handle_message(client)),
        loop.create_task(init_models()),
    ]
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        pass
    finally:
        loop.run_until_complete(close(client, tasks))
        loop.close()
