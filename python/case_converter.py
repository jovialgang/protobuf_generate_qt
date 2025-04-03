import re


def to_snake(name: str) -> str:
    name = re.sub('(.)([A-Z][a-z]+[^_]$)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', name).lower()


def to_upper_snake(name: str) -> str:
    name = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', name).upper()


def to_pascal(name: str) -> str:
    res = ""
    for word in name.split('_'):
        res += word[0].upper() + word[1:]
    return res


def to_camel(name: str) -> str:
    res = ""
    for word in name.split('_'):
        res += word[0].upper() + word[1:]
    return res[0].lower() + res[1:]
