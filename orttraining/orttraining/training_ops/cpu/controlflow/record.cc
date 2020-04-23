// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/providers/cpu/tensor/utils.h"
#include "common.h"
#include "record.h"

namespace onnxruntime {
namespace contrib {

ONNX_OPERATOR_KERNEL_EX(
    RecordEvent,
    kMSDomain,
    1,
    kCpuExecutionProvider,
    KernelDefBuilder()
        .TypeConstraint("TInt64", DataTypeImpl::GetTensorType<int64_t>())
        .TypeConstraint("T", DataTypeImpl::AllFixedSizeTensorTypes())
        .Alias(AliasRange<1, 0>(0, 1024)),
    RecordEvent);

Status RecordEvent::Compute(OpKernelContext* ctx) const {
  const Tensor* event_id_tensor = ctx->Input<Tensor>(0);
  const int64_t event_id = *event_id_tensor->template Data<int64_t>();

  ORT_RETURN_IF_NOT(event_id != -1, "-1 is reserved for skip wait, so cannot be used in RecordEvent");

  OrtEventPool::GetInstance().SignalEvent(event_id);

  for (int i_out = 0; i_out < ctx->OutputCount(); ++i_out) {
    const Tensor* X = ctx->Input<Tensor>(i_out + 1);
    const TensorShape& data_shape = X->Shape();
    Tensor* Y = ctx->Output(i_out, data_shape);
    CopyCpuTensor(X, Y);
  }
  return Status::OK();
}

}  // namespace contrib
}  // namespace onnxruntime
