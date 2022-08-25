import logging

class Module():
    
    log: logging.Logger

    def __init__(self, name: str, log_level: int = logging.INFO):
        ch = logging.StreamHandler()
        formatter = logging.Formatter(
            '|%(levelname)7s| %(name)22s | %(message)s')
        ch.setFormatter(formatter)
        self.log = logging.getLogger(name)
        self.log.setLevel(log_level)
        self.log.addHandler(ch)