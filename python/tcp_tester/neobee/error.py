class NeoBeeError(Exception):
    pass


class NotConnectedError(NeoBeeError):
    pass


class DataError(NeoBeeError):
    pass


class AlreadyConnectedError(NeoBeeError):
    pass


class BadRequestError(NeoBeeError):
    pass


class BadMethodError(NeoBeeError):
    pass


class WrongResponseCommandError(NeoBeeError):
    pass


class NetworkError(NeoBeeError):
    pass
