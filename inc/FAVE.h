#include "external/glad/glad.h"

#include "logging/logger.h"
#include "logging/exceptions.h"

#include "util/file_utils.h"
#include "util/time_provider.h"

#include "base/definitions.h"
#include "base/indexable.h"

#include "buffers/buffer.h"
#include "buffers/vbo.h"
#include "buffers/vao.h"
#include "buffers/ebo.h"

#include "materials/shader.h"
#include "materials/texture.h"

#include "objects/object.h"
#include "objects/render_object.h"
#include "objects/mesh.h"
#include "objects/fluid_simulation.h"
#include "objects/fluid_simulation_2D.h"
// #include "objects/fluid_simulation_3D.h"
#include "objects/light.h"
#include "objects/camera.h"

#include "scripting/script.h"

#include "core/scene.h"
#include "core/window.h"
#include "core/core.h"

#include "scripting/camera_controller.h"