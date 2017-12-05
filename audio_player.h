/** -*- coding: utf-8; mode: c++ -*-
 * Copyright 2017 Masashi Fujimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once


#include <sndfile.hh>
#include <portaudiocpp/PortAudioCpp.hxx>
#include <memory>

class audio_player
{
public:
  audio_player(void)
    : stream_(),
      handle_(),
      position_(0),
      loop_(false)
  {
  }

  ~audio_player(void)
  {
    this->close();
  }

  bool
  open(const char* filename)
  {
    this->close();

    this->handle_ = std::make_unique<SndfileHandle>(filename);
    this->stream_ = std::make_unique<portaudio::MemFunCallbackStream<audio_player>>(this->make_parameters(),
                                                                                    *this,
                                                                                    &audio_player::output);
    return (this->stream_) ? true : false;
  }

  bool
  start(void)
  {
    if (this->stream_)
      {
        this->stream_->start();
        return true;
      }
    return false;
  }

  void
  stop(void)
  {
    if (this->stream_)
      {
        this->stream_->stop();
      }
    this->position_ = 0;
  }

  void
  close(void)
  {
    this->stop();
    this->stream_.reset();
    this->handle_.reset();
  }

  constexpr bool
  is_active(void) const noexcept
  {
    return ((this->stream_) && (this->stream_->isActive()));
  }

  constexpr bool
  loop(void) const noexcept
  {
    return this->loop_;
  }

  constexpr void
  loop(bool value) noexcept
  {
    this->loop_ = value;
  }

private:
  int
  output(const void* input_buffer, void* output_buffer, unsigned long num_frames, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags)
  {
    static_cast<void>(input_buffer);
    static_cast<void>(time_info);
    static_cast<void>(flags);

    if (!this->handle_)
      {
        return  paAbort;
      }

    this->handle_->seek(this->position_,
                        SEEK_SET);
    auto count = this->handle_->readf(static_cast<float*>(output_buffer),
                                      static_cast<sf_count_t>(num_frames));
    this->position_ += count;
    if (this->position_ >= this->handle_->frames())
      {
        if (this->loop())
          {
            this->position_ = 0;
          }
        else
          {
            return paComplete;
          }
      }

    return paContinue;
  }

  portaudio::StreamParameters
  make_parameters(void)
  {
    auto params = portaudio::StreamParameters();
    params.setInputParameters(portaudio::DirectionSpecificStreamParameters::null());

    auto output_params = portaudio::DirectionSpecificStreamParameters();
    output_params.setNumChannels(this->handle_->channels());
    output_params.setSampleFormat(portaudio::SampleDataFormat::FLOAT32);
    output_params.setDevice(portaudio::System::instance().defaultOutputDevice());
    output_params.setHostApiSpecificStreamInfo(nullptr);
    output_params.setSuggestedLatency(portaudio::System::instance().defaultOutputDevice().defaultLowOutputLatency());
    params.setOutputParameters(output_params);

    params.setSampleRate(this->handle_->samplerate());
    params.setFramesPerBuffer(64);
    params.clearFlags();

    return params;
  }

private:
  std::unique_ptr<portaudio::Stream> stream_;
  std::unique_ptr<SndfileHandle> handle_;
  sf_count_t position_;
  bool loop_;
};
