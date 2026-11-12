#pragma once

#include "dxvk_gaze_map.h"
#include "dxvk_gaze_shm.h"
#include <chrono>
#include <string>
#include <vector>

namespace dxvk
{
  // Owned by the map worker. Mapping, heartbeat checks, gaze transforms,
  // fake gaze and map generation never touch the rendering thread.
  class DxvkGazeSource
  {
  public:
    DxvkGazeSource();
    ~DxvkGazeSource();
    DxvkGazeSource(const DxvkGazeSource &) = delete;
    DxvkGazeSource &operator=(const DxvkGazeSource &) = delete;
    bool generate(const GazeMapRequest &pass, GazeMap &result);

  private:
    using GazeShm = gaze_bridge::SharedGaze;
    struct Gaze
    {
      float cx[2] = {}, cy[2] = {};
      int32_t flipX[2] = {}, flipY[2] = {};
      uint32_t eyeMask = 3;
    };
    void *m_file = nullptr, *m_map = nullptr;
    const GazeShm *m_shm = nullptr;
    std::chrono::steady_clock::time_point m_nextOpen = {};
    gaze_bridge::Freshness m_sourceFreshness;
    bool m_debug = false, m_flipY = false, m_swapEyes = false;
    int m_forceFlipX[2] = {-1, -1}, m_forceFlipY[2] = {-1, -1};
    float m_gainY = 1.0f, m_fixedY = 0.25f;
    int m_fake = 0;
    std::string m_path;
    std::vector<std::string> m_paths;
    uint32_t m_nextPath = 0u;
    bool m_loggedNoGaze = false, m_lastValid = false;
    GazeMapRequest m_lastRequest;
    float m_winMin[2] = {1.0f, 1.0f}, m_winMax[2] = {0.0f, 0.0f};
    float m_winMinX[2] = {1.0f, 1.0f}, m_winMaxX[2] = {0.0f, 0.0f};
    std::chrono::steady_clock::time_point m_nextGazeLog = {};

    // Camera-tracking specific tuning. All of this runs on the map worker.
    float m_holdMs = 150.0f;    // reuse the last valid packet through dropouts
    float m_smoothMs = 40.0f;   // smoothing time constant, 0 = off
    float m_predictMs = 0.0f;   // velocity extrapolation, 0 = off
    float m_offsetX[2] = {}, m_offsetY[2] = {};
    float m_scaleX[2] = {1.0f, 1.0f}, m_scaleY[2] = {1.0f, 1.0f};
    uint32_t m_deadband = 1u;   // tiles of gaze movement ignored

    // Last packet used, kept for dropout hold.
    Gaze m_lastGaze = {};
    std::chrono::steady_clock::time_point m_lastGazeTime = {};
    bool m_haveLastGaze = false;

    // Smoothing/prediction state.
    Gaze m_rawGaze = {};
    Gaze m_smoothGaze = {};
    std::chrono::steady_clock::time_point m_smoothTime = {};
    bool m_haveSmooth = false;

    void openSource();
    void closeSource();
    bool readGaze(Gaze &g);
    void tune(Gaze &g, bool fake);
  };
}
