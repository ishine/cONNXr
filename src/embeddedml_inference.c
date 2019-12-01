#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pb/onnx.pb-c.h"
#include "embeddedml_utils.h"
#include "embeddedml_debug.h"
#include "embeddedml_inference.h"

int _outputIdx = 0;
Onnx__TensorProto *_outputs[MAX_NUM_OF_OUTPUTS] = {};

// TODO Move somewhere?
static void call_operator(const char *name,
                          size_t n_input,
                          Onnx__TensorProto **input,
                          size_t n_attribute,
                          Onnx__AttributeProto **attribute,
                          size_t n_output,
                          Onnx__TensorProto **output)
{
  int i;
  for(i = 0; operatorsSet[i].name; i++) {
    if(strcmp(operatorsSet[i].name, name) == 0) {
      operatorsSet[i].func(n_input,
                           input,
                           n_attribute,
                           attribute,
                           n_output,
                           output);
      return;
    }
  }
  // If it reaches this point, the operator wasnt found
  operator_notimplemented(name);
}

Onnx__TensorProto** inference(Onnx__ModelProto *model, Onnx__TensorProto **inputs, int nInputs)
{
  //int error = 0;

  /* Dirty trick to allow multiple runs. There is a memory leak for sure */
  _outputIdx = 0;
  DEBUG_PRINT("calling inference");
  DEBUG_PRINT("The graph has nodes=%zu", model->graph->n_node);

  // Iterate all nodes in the graph
  for (int nodeIdx = 0; nodeIdx < model->graph->n_node; nodeIdx++)
  {
    char *operation = model->graph->node[nodeIdx]->op_type;
    DEBUG_PRINT("node=%d, operation=%s, n_input=%zu, n_output=%zu",
                nodeIdx,
                model->graph->node[nodeIdx]->op_type,
                model->graph->node[nodeIdx]->n_input,
                model->graph->node[nodeIdx]->n_output);

    // TODO hardcoded to one output
    size_t nOutputs = 1;
    Onnx__TensorProto *out0 = malloc(sizeof(*out0));
    Onnx__TensorProto **nodeOutputs = malloc(sizeof(*out0));
    nodeOutputs[0] = out0;
    Onnx__TensorProto **nodeInputs = malloc(sizeof(*out0) * model->graph->node[nodeIdx]->n_input);

    // Populate the input array by gathering all the required inputs
    for (int inp = 0; inp < model->graph->node[nodeIdx]->n_input; ++inp) {
      Onnx__TensorProto *inpN =malloc(sizeof(*inpN));
      inpN = searchTensorProtoByName(model, inputs, nInputs, model->graph->node[nodeIdx]->input[inp]);
      nodeInputs[inp] = inpN;
    }

    call_operator(operation,
                  model->graph->node[nodeIdx]->n_input,
                  nodeInputs,
                  model->graph->node[nodeIdx]->n_attribute,
                  model->graph->node[nodeIdx]->attribute,
                  nOutputs,// TODO use model->graph->node[nodeIdx]->n_output
                  nodeOutputs);

    // TODO temporal shit
    printf("deug\n\n");
    for (int i = 0; i < nodeOutputs[0]->n_float_data; i++) {
      //printf("float_data[%d] = %f\n", i, nodeOutputs[0]->float_data[i]);
    }
    out0->name = model->graph->node[nodeIdx]->output[0];
    DEBUG_PRINT("Storing output in list index=%d, name=%s", _outputIdx, out0->name);
    _outputs[_outputIdx++] = nodeOutputs[0]; // todo this is hardcoded
    printf("_outputIdx = %d\n", _outputIdx);
  }

  // TODO:
  // Free calculaterTensors memory
  // Free also extra allocations within the structure (i.e. doubles...)

  return _outputs;
}
