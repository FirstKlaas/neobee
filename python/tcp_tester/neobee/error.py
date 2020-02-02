class NeoBeeError(Exception):
    pass


class NotConnectedError(NeoBeeError):
    pass


class AlreadyConnectedError(NeoBeeError):
    pass


class BadRequestError(NeoBeeError):
    pass


class WrongResponseCommandError(NeoBeeError):
    pass


class NetworkError(NeoBeeError):
    pass

