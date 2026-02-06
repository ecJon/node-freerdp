#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#else
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#endif

#include "rdp.h"

#include "generator.h"
#include "context.h"
// TODO: Clipboard needs to be rewritten for FreeRDP 3.x API
// #include "cliprdr.h"

#include <errno.h>
#include <freerdp/client.h>  // FreeRDP 3.x client API
#include <stdio.h>
#include <string.h>

#include <nan.h>

#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/client.h>
#include <freerdp/settings.h>
#include <freerdp/channels/channels.h>
#include <freerdp/codec/color.h>

#include <winpr/crt.h>
#include <winpr/synch.h>

#include <nan.h>

using Nan::Callback;

using v8::Object;
using v8::Array;
using v8::Number;
using v8::Value;
using v8::Local;
using v8::String;
using Nan::New;
using Nan::Null;

struct thread_data
{
  freerdp* instance;
  bool stopping;
};

thread_data** sessions;
int sessionCount = 0;

bool node_freerdp_global_init = false;

int add_session(thread_data* session)
{
  if(sessions == NULL) {
    sessionCount = 1;
    sessions = (thread_data **)malloc(sizeof(thread_data *));
  } else {
    sessionCount += 1;
    thread_data **newSessions = (thread_data **)malloc(sizeof(thread_data *) * sessionCount);
    memcpy(newSessions, sessions, sizeof(thread_data *) * (sessionCount - 1));
    free(sessions);
    sessions = newSessions;
  }

  sessions[sessionCount - 1] = session;

  return sessionCount - 1;
}

BOOL node_context_new(freerdp* instance, rdpContext* context)
{
  // In FreeRDP 3.x, channels are automatically initialized with the context
  // No need to call freerdp_channels_new() anymore
  return TRUE;
}

void node_context_free(freerdp* instance, rdpContext* context)
{

}


BOOL node_begin_paint(rdpContext* context)
{
  rdpGdi* gdi = context->gdi;
  gdi->primary->hdc->hwnd->invalid->null = 1;
  return TRUE;
}

struct connect_args {};

Local<Array> connect_args_parser(void *generic) {
  connect_args *args = static_cast<connect_args *>(generic);

  Local<Array> argv = New<Array>();

  free(args);

  return argv;
}

const struct GeneratorType CONNECT_GENERATOR_TYPE {
  .name = "connect",
  .arg_parser = connect_args_parser
};

Local<Array> close_args_parser(void *generic) {
  connect_args *args = static_cast<connect_args *>(generic);

  Local<Array> argv = New<Array>();

  free(args);

  return argv;
}

struct close_args {};

const struct GeneratorType CLOSE_GENERATOR_TYPE {
  .name = "close",
  .arg_parser = close_args_parser
};

struct draw_args {
  int x;
  int y;
  int w;
  int h;
  int bpp;
  BYTE* buffer;
};

Local<Array> draw_args_parser(void *generic) {
  draw_args *args = static_cast<draw_args *>(generic);

  Local<Object> obj = New<Object>();

  Nan::Set(obj, New<String>("x").ToLocalChecked(), New<Number>(args->x));
  Nan::Set(obj, New<String>("y").ToLocalChecked(), New<Number>(args->y));
  Nan::Set(obj, New<String>("w").ToLocalChecked(), New<Number>(args->w));
  Nan::Set(obj, New<String>("h").ToLocalChecked(), New<Number>(args->h));
  Nan::Set(obj, New<String>("bpp").ToLocalChecked(), New<Number>(args->bpp));

  int size = args->w * args->h * args->bpp;

  Nan::MaybeLocal<v8::Object> buffer = Nan::CopyBuffer((const char *)args->buffer, size);
  Nan::Set(obj, New<String>("buffer").ToLocalChecked(), buffer.ToLocalChecked());

  Local<Array> argv = New<Array>();
  Nan::Set(argv, 0, obj);

  delete[] args->buffer;
  delete args;

  return argv;
}

const struct GeneratorType DRAW_GENERATOR_TYPE {
  .name = "bitmap",
  .arg_parser = draw_args_parser
};

BOOL node_end_paint(rdpContext* context)
{
  rdpGdi* gdi = context->gdi;
  if (gdi->primary->hdc->hwnd->invalid->null)
    return TRUE;

  draw_args *args = new draw_args;
  args->x = gdi->primary->hdc->hwnd->invalid->x;
  args->y = gdi->primary->hdc->hwnd->invalid->y;
  args->w = gdi->primary->hdc->hwnd->invalid->w;
  args->h = gdi->primary->hdc->hwnd->invalid->h;

  // FreeRDP 3.x: Calculate bytes per pixel from format
  // For PIXEL_FORMAT_ABGR32, bpp is 4 bytes
  args->bpp = 4;

  int size = args->w * args->h * args->bpp;
  args->buffer = new BYTE[size];

  // copy only lines relevant to updated chunk
  int dest_pos = 0;
  int dest_line_width = args->w * args->bpp;
  for(int i = args->y; i < args->y + args->h; i++) {
    // memcopy only columns that are relevant
    int start_pos = (i * gdi->width * args->bpp) + (args->x * args->bpp);
    BYTE* src = &gdi->primary_buffer[start_pos];
    BYTE* dest = &args->buffer[dest_pos];
    memcpy(dest, src, dest_line_width);
    dest_pos += dest_line_width;
  }

  nodeContext *nc = (nodeContext*)context;

  generator_emit(nc->generatorContext, &DRAW_GENERATOR_TYPE, args);

  return TRUE;
}

BOOL node_receive_channel_data(freerdp* instance, UINT16 channelId, const BYTE* data, size_t size, UINT32 flags, size_t total_size)
{
  return freerdp_channels_data(instance, channelId, data, size, flags, total_size);
}

void node_process_channel_event(rdpChannels* channels, freerdp* instance)
{
  // FreeRDP 3.x: freerdp_channels_pop_event is removed
  // Events are now processed differently through callbacks
  // For now, we skip this function as it's no longer needed
  // The clipboard events will be handled through registered callbacks
}

BOOL node_pre_connect(freerdp* instance)
{
  nodeInfo* nodei;
  nodeContext* context;
  rdpSettings* settings;

  context = (nodeContext*) instance->context;

  nodei = (nodeInfo*) malloc(sizeof(nodeInfo));
  ZeroMemory(nodei, sizeof(nodeInfo));

  context->nodei = nodei;

  settings = instance->context->settings;

  // FreeRDP 3.x: OrderSupport is deprecated and should not be used
  // The orders are now managed internally

  // freerdp_channels_pre_connect is no longer needed in 3.x
  // Channels are initialized automatically

  // Disable graphics decoding - this might help with the hanging issue
  // Similar to what the sample client does
  freerdp_settings_set_bool(settings, FreeRDP_DeactivateClientDecoding, TRUE);

  return TRUE;
}

BOOL node_post_connect(freerdp* instance)
{
  printf("DEBUG: node_post_connect called\n");

  rdpGdi* gdi;

  // FreeRDP 3.x: Use PIXEL_FORMAT_* instead of CLRCONV_* constants
  gdi_init(instance, PIXEL_FORMAT_ABGR32);
  gdi = instance->context->gdi;

  // FreeRDP 3.x: update is in context, not in instance
  instance->context->update->BeginPaint = node_begin_paint;
  instance->context->update->EndPaint = node_end_paint;

  // TODO: Clipboard needs to be rewritten for FreeRDP 3.x API
  // node_cliprdr_init(instance);

  // freerdp_channels_post_connect is no longer needed in 3.x
  // Channels are initialized automatically

  printf("DEBUG: Emitting connect event\n");
  nodeContext *nc = (nodeContext*)instance->context;
  connect_args *args = (connect_args *)malloc(sizeof(connect_args));
  generator_emit(nc->generatorContext, &CONNECT_GENERATOR_TYPE, args);

  printf("DEBUG: node_post_connect completed\n");
  return TRUE;
}

int tfreerdp_run(thread_data* data)
{
  HANDLE events[64];
  DWORD count = 0;
  DWORD status;
  rdpChannels* channels;

  freerdp* instance = data->instance;
  channels = instance->context->channels;

  // FreeRDP 3.x: Start client before connecting
  printf("DEBUG: Calling freerdp_client_start...\n");
  if (freerdp_client_start(instance->context) != 0) {
    printf("Failed to start client\n");
    return 1;
  }

  printf("DEBUG: Calling freerdp_connect...\n");
  BOOL connected = freerdp_connect(instance);
  printf("DEBUG: freerdp_connect returned: %d\n", connected);

  if (!connected) {
    printf("DEBUG: Connection failed!\n");
    freerdp_client_stop(instance->context);
    return 1;
  }

  printf("DEBUG: Connection established, entering event loop\n");
  while (!data->stopping)
  {
    // FreeRDP 3.x: Use event handles API (count is a value, not pointer)
    count = freerdp_get_event_handles(instance->context, events, 64);
    if (count == 0)
    {
      printf("Failed to get event handles\n");
      break;
    }

    status = WaitForMultipleObjects(count, events, FALSE, 100);
    if (status == WAIT_FAILED)
    {
      printf("WaitForMultipleObjects failed\n");
      break;
    }

    // FreeRDP 3.x: Use event check API
    if (!freerdp_check_event_handles(instance->context))
    {
      printf("Failed to check event handles\n");
      break;
    }

    node_process_channel_event(channels, instance);

    if (freerdp_shall_disconnect_context(instance->context))
    {
      printf("Connection closed\n");
      break;
    }
  }

  nodeContext *nc = (nodeContext*)instance->context;
  close_args *args = (close_args *)malloc(sizeof(close_args));
  generator_emit(nc->generatorContext, &CLOSE_GENERATOR_TYPE, args);

  // TODO: Clipboard needs to be rewritten for FreeRDP 3.x API
  // node_cliprdr_uninit(instance);

  free(nc->generatorContext);

  // FreeRDP 3.x: Use client API for proper cleanup
  freerdp_client_stop(instance->context);
  freerdp_client_context_free(instance->context);

  return 0;
}

void* thread_func(void* param)
{
  struct thread_data* data;
  data = (struct thread_data*) param;

  tfreerdp_run(data);

  free(data);

  pthread_detach(pthread_self());

  return NULL;
}

// FreeRDP 3.x client callbacks
static BOOL node_client_new(freerdp* instance, rdpContext* context)
{
  // Initialize our custom context
  nodeContext* nContext = (nodeContext*)context;

  // Set up callbacks
  instance->PreConnect = node_pre_connect;
  instance->PostConnect = node_post_connect;
  instance->ReceiveChannelData = node_receive_channel_data;

  return TRUE;
}

static void node_client_free(freerdp* instance, rdpContext* context)
{
  // Cleanup handled in node_context_free
}

static int node_client_start(rdpContext* context)
{
  // Client-specific initialization if needed
  return 0;
}

static int node_client_stop(rdpContext* context)
{
  // Client-specific cleanup if needed
  return 0;
}

int node_freerdp_connect(int argc, char* argv[], Callback *callback)
{
  int status;
  pthread_t thread;
  rdpContext* context;
  struct thread_data* data;
  nodeContext* nContext;

  if (!node_freerdp_global_init) {
    node_freerdp_global_init = true;
  }

  // FreeRDP 3.x: Use client API with RDP_CLIENT_ENTRY_POINTS_V1
  RDP_CLIENT_ENTRY_POINTS_V1 entryPoints = { 0 };

  entryPoints.Size = sizeof(RDP_CLIENT_ENTRY_POINTS_V1);
  entryPoints.Version = RDP_CLIENT_INTERFACE_VERSION;

  // Settings will be created by freerdp_client_context_new
  entryPoints.settings = nullptr;

  entryPoints.GlobalInit = nullptr;
  entryPoints.GlobalUninit = nullptr;

  entryPoints.ContextSize = sizeof(nodeContext);
  entryPoints.ClientNew = node_client_new;
  entryPoints.ClientFree = node_client_free;
  entryPoints.ClientStart = node_client_start;
  entryPoints.ClientStop = node_client_stop;

  // Create context using client API
  context = freerdp_client_context_new((RDP_CLIENT_ENTRY_POINTS*)&entryPoints);
  if (!context) {
    printf("Failed to create client context\n");
    return -1;
  }

  nContext = (nodeContext*)context;
  nContext->generatorContext = new GeneratorContext;
  nContext->generatorContext->callback = callback;

  // Parse command line arguments
  status = freerdp_client_settings_parse_command_line(
    context->settings,
    argc,
    argv,
    FALSE  // allowUnknown
  );

  if (status < 0) {
    freerdp_client_context_free(context);
    exit(0);
  }

  data = (struct thread_data*) malloc(sizeof(struct thread_data));
  ZeroMemory(data, sizeof(struct thread_data));

  data->instance = context->instance;
  data->stopping = false;

  pthread_create(&thread, 0, thread_func, data);

  int index = add_session(data);

  return index;
}

void node_freerdp_send_key_event_scancode(int session_index, int code, int pressed)
{
  thread_data* session = sessions[session_index];
  freerdp* instance = session->instance;
  // FreeRDP 3.x: input is in context
  rdpInput* input = instance->context->input;

  // FreeRDP 3.x: freerdp_input_send_keyboard_event_ex requires 4 parameters
  freerdp_input_send_keyboard_event_ex(input, pressed, FALSE, code);
}

void node_freerdp_send_pointer_event(int session_index, int flags, int x, int y)
{
  thread_data* session = sessions[session_index];
  freerdp* instance = session->instance;
  // FreeRDP 3.x: input is in context
  rdpInput* input = instance->context->input;

  input->MouseEvent(input, flags, x, y);
}

void node_freerdp_set_clipboard(int session_index, void* data, int len)
{
  // TODO: Clipboard needs to be rewritten for FreeRDP 3.x API
  // For now, clipboard functionality is disabled
}

void node_freerdp_close(int session_index)
{
  // NOTE: Doesn't block on closed session, will send closed event when completed
  thread_data *session = sessions[session_index];
  session->stopping = true;
}
