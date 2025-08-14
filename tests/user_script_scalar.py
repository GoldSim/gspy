def goldsim_calculate(inputs):
    # Expects inputs: {'a': float, 'b': float}
    a = inputs.get('a', 0.0)
    b = inputs.get('b', 0.0)
    return {'result': a + b}
