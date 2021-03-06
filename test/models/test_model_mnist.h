#ifndef TEST_MODEL_MNIST_H
#define TEST_MODEL_MNIST_H

#include "common_models.h"

void test_model_mnist(void)
{
  // Use mnist model to test
  // test_data_set_0
  // test_data_set_1
  // test_data_set_2

  TRACE_LEVEL0("Start: test_model_mnist");

  Onnx__ModelProto *model = openOnnxFile("test/mnist/model.onnx");
  Onnx__TensorProto *inp0set0 = openTensorProtoFile("test/mnist/test_data_set_0/input_0.pb");
  Onnx__TensorProto *out0set0 = openTensorProtoFile("test/mnist/test_data_set_0/output_0.pb");

  //Debug_PrintModelInformation(model);
  debug_prettyprint_model(model);
  convertRawDataOfTensorProto(inp0set0);
  convertRawDataOfTensorProto(out0set0);

  // TODO Dirty trick. I expected the input name to be included in the
  // input_0, but apparently it is not. Dont know if memory for the name
  // is allocated... but it doesnt crash
  inp0set0->name = "Input3";
  printf("%s\n\n", inp0set0->name);

  Onnx__TensorProto *inputs[] = { inp0set0 };
  clock_t start, end;
  double cpu_time_used;

  start = clock();
  Onnx__TensorProto **output = inference(model, inputs, 1);
  end = clock();

  /* TODO: Could be nice to use model->graph->name if we can be sure that
   * all modelas have a unique and meaningful name. In this case [mnist] no
   * longer needs to be hardcoded.
   */
  // TODO Is CLOCKS_PER_SEC ok to use?
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("[benchmark][mnist] cycles: %f\n", (double) (end - start));
  printf("[benchmark][mnist] cpu_time_used: %f\n", cpu_time_used);
  printf("[benchmark][mnist] CLOCKS_PER_SEC: %d\n", CLOCKS_PER_SEC);

  /* 11 is hardcoded, which is Plus214_Output_0 */
  compareAlmostEqualTensorProto(output[11], out0set0);

  TRACE_LEVEL0("End: test_model_mnist");
}

static void test_model_mnist_node(char *outputName)
{
  char modelPath[200];
  strcpy(modelPath, "test/mnist_experimental_test/");
  strcat(modelPath, outputName);
  strcat(modelPath, "/");
  strcat(modelPath, outputName);
  strcat(modelPath, ".onnx");

  char inputPath[200];
  strcpy(inputPath, "test/mnist_experimental_test/");
  strcat(inputPath, outputName);
  strcat(inputPath, "/test_data_set_0/input_0.pb");

  char outputPath[200];
  strcpy(outputPath, "test/mnist_experimental_test/");
  strcat(outputPath, outputName);
  strcat(outputPath, "/test_data_set_0/output_0.pb");

  printf("Reading model %s\n", modelPath);

  Onnx__ModelProto *model = openOnnxFile(modelPath);

  /* Dirty trick. The nodes inside mnist_experimental_test are reversed
  dont know why. Just swaps. TODO:*/

  Onnx__NodeProto *temp0 = model->graph->node[0];
  Onnx__NodeProto *temp3 = model->graph->node[3];
  //Onnx__NodeProto *temp2 = model->graph->node[2];
  Onnx__NodeProto *temp1 = model->graph->node[1];
  model->graph->node[0] = model->graph->node[4];
  model->graph->node[1] = temp3;
  //model->graph->node[2] = temp1;
  model->graph->node[3] = temp1;
  model->graph->node[4] = temp0;


  debug_prettyprint_model(model);

  Onnx__TensorProto *inp0set0 = openTensorProtoFile(inputPath);
  Onnx__TensorProto *out0set0 = openTensorProtoFile(outputPath);
  convertRawDataOfTensorProto(inp0set0);
  convertRawDataOfTensorProto(out0set0);
  inp0set0->name = "Input3";
  Onnx__TensorProto *inputs[] = { inp0set0 };
  Onnx__TensorProto **output = inference(model, inputs, 1);

  int outputToAssert = 4;
  TRACE_LEVEL0("Asserting output %s", output[outputToAssert]->name);
  compareAlmostEqualTensorProto(output[outputToAssert], out0set0);
}

void test_model_mnist_per_node(void)
{
  /* This tests are not part of the official mnist onnx model. They are generated
  with a python script which generates one model +input/output per node. This
  can be used to detect problems in a specific node within a model
  Autogenerated models are:
  -0_Parameter193_reshape1
  -1_Convolution28_Output_0
  -2_Plus30_Output_0
  -3_ReLU32_Output_0
  -4_Pooling66_Output_0
  -5_Convolution110_Output_0
  -6_Plus112_Output_0
  -7_ReLU114_Output_0
  -8_Pooling160_Output_0
  -9_Pooling160_Output_0_reshape0
  -10_Times212_Output_0
  -11_Plus214_Output_0
  */
  //TRACE_LEVEL0("Start: test_model_mnist_per_node");

  //test_model_mnist_node("0_Parameter193_reshape1");

  /* Fails in the convolution operator, hence all next ones also fail */
  //test_model_mnist_node("1_Convolution28_Output_0");


  //test_model_mnist_node("2_Plus30_Output_0");
  //test_model_mnist_node("3_ReLU32_Output_0");
  //test_model_mnist_node("4_Pooling66_Output_0");
  test_model_mnist_node("5_Convolution110_Output_0");
  //test_model_mnist_node("6_Plus112_Output_0");
  //test_model_mnist_node("7_ReLU114_Output_0");
  //test_model_mnist_node("8_Pooling160_Output_0");
  //test_model_mnist_node("9_Pooling160_Output_0_reshape0");
  //test_model_mnist_node("10_Times212_Output_0");
  //test_model_mnist_node("11_Plus214_Output_0");

}

#endif
