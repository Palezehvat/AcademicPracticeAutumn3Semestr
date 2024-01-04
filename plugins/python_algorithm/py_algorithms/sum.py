import json

class class_sum:
    def __init__(self, dictionary):
        json_from_string = json.loads(dictionary)
        self.first_number = int(json_from_string["config"]["a"])
        self.second_number = int(json_from_string["config"]["b"])

    def run(self, dict):
        thisdict = {
            "parametr1": ("uint32_t", abs(self.first_number + self.second_number))
        }
        return thisdict