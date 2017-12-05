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

#include "audio_player.h"
#include "3rdparty/emacs/emacs-module.h"

#include <vector>
#include <array>

int plugin_is_GPL_compatible;

namespace
{

portaudio::AutoSystem auto_system;
audio_player player;

emacs_value
play_audio(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * user_data) noexcept
{
  auto player = reinterpret_cast<audio_player*>(user_data);
  if (player == nullptr)
    {
      return env->intern(env,
                         "nil");
    }

  if (nargs != 1)
    {
      return env->intern(env,
                         "nil");
    }

  // emacs_valueから文字列を抽出する方法.
  // 1. まず文字列の長さを取得.
  ptrdiff_t size(0);
  env->copy_string_contents(env,
                            args[0], // より厳密には, env->type_of()で型をチェックした方が良い
                            nullptr,
                            &size);
  // 2. その長さが格納できるバッファを作り,
  std::vector<char> buffer(size);
  // 3. 文字列を取得.
  auto successed = env->copy_string_contents(env,
                                             args[0],
                                             &buffer[0],
                                             &size);
  if (successed == false)
    {
      return env->intern(env,
                         "nil");
    }

  auto result = player->open(&buffer[0]);
  if (result)
    {
      player->start();
    }
  return env->intern(env,
                     (result) ? "t" : "nil");
}

emacs_value
stop_audio(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * user_data) noexcept
{
  static_cast<void>(nargs);
  static_cast<void>(args);

  auto player = reinterpret_cast<audio_player*>(user_data);
  if (player == nullptr)
    {
      return env->intern(env,
                         "nil");
    }

  player->close();

  return env->intern(env,
                     (player->is_active()) ? "nil" : "t");
}

emacs_value
is_active_audio(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * user_data) noexcept
{
  static_cast<void>(nargs);
  static_cast<void>(args);

  auto player = reinterpret_cast<audio_player*>(user_data);
  if (player == nullptr)
    {
      return env->intern(env,
                         "nil");
    }

  return env->intern(env,
                     (player->is_active()) ? "t" : "nil");
}

emacs_value
loop_audio(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * user_data) noexcept
{
  auto player = reinterpret_cast<audio_player*>(user_data);
  if (player == nullptr)
    {
      return env->intern(env,
                         "nil");
    }

  if (nargs != 1)
    {
      return env->intern(env,
                         "nil");
    }

  player->loop(env->is_not_nil(env,
                               args[0]));
  return env->intern(env,
                     "t");
}


///
struct emacs_symbol_property
{
  using function_t = emacs_value (*)(emacs_env* env, ptrdiff_t nargs, emacs_value args[], void*) noexcept;
  std::string symbol_name;
  std::string documentation;
  ptrdiff_t min_arity;
  ptrdiff_t max_arity;
  function_t function;
}; // struct emacs_symbol_property

std::vector<emacs_symbol_property>
make_emacs_symbol_properties(void)
{
  std::vector<emacs_symbol_property> properties;

  emacs_symbol_property property;

  property.symbol_name = "audio-player-play-audio";
  property.documentation = "Audio player plays the audio `file-path'.";
  property.min_arity = 1;
  property.max_arity = 1;
  property.function = play_audio;
  properties.push_back(property);

  property.symbol_name = "audio-player-stop-audio";
  property.documentation = "Audio player stop playing audio.";
  property.min_arity = 0;
  property.max_arity = 0;
  property.function = stop_audio;
  properties.push_back(property);

  property.symbol_name = "audio-player-is-active-audio";
  property.documentation = "Is audio player active?";
  property.min_arity = 0;
  property.max_arity = 0;
  property.function = is_active_audio;
  properties.push_back(property);

  property.symbol_name = "audio-player-loop-audio";
  property.documentation = "Audio player repeatedly plays audio.";
  property.min_arity = 1;
  property.max_arity = 1;
  property.function = loop_audio;
  properties.push_back(property);

  return properties;
}

void
message(emacs_env* env, const std::string& text)
{
  auto message = env->intern(env,
                             "message");
  auto str = env->make_string(env,
                              text.c_str(),
                              text.size());
  std::array<emacs_value, 1> args{{str}};
  env->funcall(env,
               message,
               args.size(),
               args.data());
}

void
fset(emacs_env* env, emacs_value symbol, emacs_value function)
{
  auto fset = env->intern(env,
                          "fset");
  std::array<emacs_value, 2> args{{symbol,
                                   function}};
  // fset symbol definition
  // https://www.gnu.org/software/emacs/manual/html_node/elisp/Function-Cells.html
  env->funcall(env,
               fset,
               args.size(),
               args.data());
}

void
provide(emacs_env* env, const std::string& feature_name)
{
  auto provide = env->intern(env,
                             "provide");
  auto feature = env->intern(env,
                             feature_name.c_str());
  std::array<emacs_value, 1> args{{feature}};
  env->funcall(env,
               provide,
               args.size(),
               args.data());
}

} // namespace

int
emacs_module_init(emacs_runtime* ert)
{
  // 1. runtimeからenvironmentを取得する
  // このenvironmentはこのDynamic Moduleの環境を意味する
  auto env = ert->get_environment(ert);

  // 2. Emacsから利用したい関数を登録する.
  std::vector<emacs_value> symbol_list;
  std::vector<emacs_value> function_list;
  auto properties = make_emacs_symbol_properties();
  for (const auto& property: properties)
    {
      function_list.emplace_back(env->make_function(env,
                                                    property.min_arity,
                                                    property.max_arity,
                                                    property.function,
                                                    property.documentation.c_str(),
                                                    &player));
      symbol_list.emplace_back(env->intern(env,
                                           property.symbol_name.c_str()));
    }
  // シンボルと関数を紐付ける
  for (auto i = 0ul; i < symbol_list.size(); ++i)
    {
      fset(env,
           symbol_list[i],
           function_list[i]);
    }

  // featuresをprovideしてEmacsから利用できるようにする.
  provide(env,
          "audio-player");
  return 0;
}

