def fail_with_exception():
    raise ValueError("This is a test exception")

def return_string():
    return "not a double"

def return_scalar_for_array():
    return 42.0

def return_non_numeric_array():
    return [1.0, "text", 3.0]

def malformed_timeseries_missing_key():
    return {
        "is_calendar": True,
        "data_type": "instantaneous",
        "times": [1.0, 2.0]
        # Missing "values"
    }

def malformed_timeseries_mismatched_length():
    return {
        "is_calendar": True,
        "data_type": "instantaneous",
        "times": [1.0, 2.0],
        "values": [10.0]
    }

def malformed_lookup_table_mismatched_length():
    return {
        "dimension": 1,
        "num_points": 3,
        "independent": [1.0, 2.0, 3.0],
        "dependent": [10.0, 20.0] # Only 2 values
    }

def return_none():
    return None
