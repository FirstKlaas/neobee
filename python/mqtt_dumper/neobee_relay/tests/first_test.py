import asyncio
import logging
from typing import Generator

import pytest
from fastapi.testclient import TestClient
from tortoise import Tortoise
from tortoise.contrib.fastapi import register_tortoise

from neobee_relay.rest.server import app
import neobee_relay.orm as orm
from neobee_relay.orm import Board, BoardModel

logger = logging.getLogger(__name__)


@pytest.fixture(scope="module")
def client() -> Generator:
    register_tortoise(
        app,
        db_url="sqlite://:memory:",
        modules={"models": ["neobee_relay.models.board", "neobee_relay.models.measure"]},
        generate_schemas=True,
        add_exception_handlers=True,
    )
    with TestClient(app) as c:
        yield c


@pytest.fixture(scope="module")
def event_loop(client: TestClient) -> Generator:
    yield client.task.get_loop()


def test_create_user(client: TestClient, event_loop: asyncio.AbstractEventLoop):  # nosec
    async def add_board():
        await orm.add_board(Board(mac="m", ip="i", name="n", firmware="f"))

    response = client.get("/boards")
    assert response.status_code == 200, response.text
    logger.warning(response.json())
    boards = [Board.parse_obj(data) for data in response.json()]
    assert len(boards) == 0

    event_loop.run_until_complete(add_board())
    response = client.get("/boards")
    assert response.status_code == 200, response.text
    boards = [Board.parse_obj(data) for data in response.json()]
    assert len(boards) == 1
    assert boards[0].mac == "m"
