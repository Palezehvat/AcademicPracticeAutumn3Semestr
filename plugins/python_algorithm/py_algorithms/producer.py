import json

class producer:
    
    def __init__(self, args):
        json_from_string = json.loads(args)
        self.data = json_from_string["config"]["data"]
        self.counter = 0
        
    def run(self, args):
        self.counter += 1
        dictionary = {
            "ready" : ("bool", True),
            "counter" : ("uint32_t", self.counter),
            "data" : ("userdata_t", str(self.data))
        }
        return dictionary
        
        