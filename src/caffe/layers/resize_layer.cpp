// Copyright 2014 BVLC and contributors.

#include <vector>

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layer.hpp"
#include "caffe/vision_layers.hpp"
#include "caffe/util/math_functions.hpp"
#include "caffe/util/util_img.hpp"

namespace caffe {

template <typename Dtype>
void ResizeLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
      vector<Blob<Dtype>*>* top) {

  ResizeParameter resize_param = this->layer_param_.resize_param();

  bool is_pyramid_test = resize_param.is_pyramid_test();
  if(is_pyramid_test == false)
  {

	  this->out_height_ = resize_param.height();
	  this->out_width_ = resize_param.width();
  }
  else
  {
	  int in_height = bottom[0]->height();
	  int in_width = bottom[0]->width();
	  this->out_height_ = int (resize_param.out_height_scale() * in_height);
	  this->out_width_ = int (resize_param.out_width_scale() * in_width);
  }
  
  if(this->locs_.size() == 0)
  {
  	  for(int i=0;i<4; i++)
	  {
		  this->locs_.push_back(new Blob<Dtype>);
	  }
  }
  
  this->out_num_ = bottom[0]->num();
  this->out_channels_ = bottom[0]->channels();
  (*top)[0]->Reshape(out_num_, out_channels_, out_height_, out_width_);

  for(int i=0;i<4; ++i)
  {
	  this->locs_[i]->Reshape(1,1,out_height_, out_width_);
  }
}


template <typename Dtype>
void ResizeLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    vector<Blob<Dtype>*>* top) {

	ResizeBlob_cpu(bottom[0], (*top)[0] ,this->locs_[0],this->locs_[1],this->locs_[2],this->locs_[3]);

        ResizeParameter resize_param = this->layer_param_.resize_param();
        Dtype theMultiple = resize_param.multiple_scale();
        caffe_scal((*top)[0]->count(), theMultiple, (*top)[0]->mutable_cpu_data());
  return;
}

template <typename Dtype>
void ResizeLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down,
    vector<Blob<Dtype>*>* bottom) {

	Dtype* bottom_diff = (*bottom)[0]->mutable_cpu_diff();
	Dtype* top_diff = top[0]->mutable_cpu_diff();

	const Dtype* loc1 = this->locs_[0]->cpu_data();
	const Dtype* weight1 = this->locs_[0]->cpu_diff();
	const Dtype* loc2 = this->locs_[1]->cpu_data();
	const Dtype* weight2 = this->locs_[1]->cpu_diff();
	const Dtype* loc3 = this->locs_[2]->cpu_data();
	const Dtype* weight3 = this->locs_[2]->cpu_diff();
	const Dtype* loc4 = this->locs_[3]->cpu_data();
	const Dtype* weight4 = this->locs_[3]->cpu_diff();

	caffe::caffe_set((*bottom)[0]->count(),Dtype(0),bottom_diff);

	for(int n=0; n< this->out_num_; ++n)
	{
		for(int c = 0; c < this->out_channels_; ++c)
		{
			int bottom_diff_offset = (*bottom)[0]->offset(n,c);
			int top_diff_offset = top[0]->offset(n,c);

			for (int idx = 0; idx < this->out_height_* this->out_width_; ++idx)
			{
				bottom_diff[bottom_diff_offset + static_cast<int>(loc1[idx])] += top_diff[top_diff_offset+idx]*weight1[idx];
				bottom_diff[bottom_diff_offset + static_cast<int>(loc2[idx])] += top_diff[top_diff_offset+idx]*weight2[idx];
				bottom_diff[bottom_diff_offset + static_cast<int>(loc3[idx])] += top_diff[top_diff_offset+idx]*weight3[idx];
				bottom_diff[bottom_diff_offset + static_cast<int>(loc4[idx])] += top_diff[top_diff_offset+idx]*weight4[idx];
			}
		}
	}

}

INSTANTIATE_CLASS(ResizeLayer);

}  // namespace caffe
