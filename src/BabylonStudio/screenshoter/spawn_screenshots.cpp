#include <babylon/core/logging.h>
#include <iostream>
#include <string>
#include <babylon/core/filesystem.h>
#include <babylon/babylon_imgui/babylon_studio.h>
#include <babylon/samples/sample_spawn.h>
#include <babylon/samples/samples_info.h>
#include <nlohmann/json.hpp>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

namespace BABYLON {
namespace impl {
using namespace BABYLON::SamplesInfo;

SampleAutoRunInfo runOneSample(const std::string & exeName, const std::string &sampleName, bool flagAsync)
{
  SampleAutoRunInfo sampleRunInfo;

  std::vector<std::string> command = {exeName, "-s", sampleName, "-p"};
  if (flagAsync)
    command.push_back("-A");

  Samples::SpawnOptions spawnOptions;
  spawnOptions.MaxExecutionTimeSeconds       = 15.;
  spawnOptions.CopyOutputToMainProgramOutput = false;
  auto spawnResult                           = SpawnWaitSubProcess(command, spawnOptions);

  if (spawnResult.ExitStatus != 0) {
    BABYLON_LOG_WARN("ScreenshotAllSamples", "Subprocess has failed for sample ", sampleName);
    sampleRunInfo.unhandledExceptionStackTrace = spawnResult.StdOutErr;
    sampleRunInfo.sampleRunStatus              = SampleAutoRunStatus::unhandledException;
  }

  if (spawnResult.MaxExecutionTimePassed) {
    BABYLON_LOG_WARN("ScreenshotAllSamples", "Subprocess MaxExecutionTimePassed for sample ",
                     sampleName);
    sampleRunInfo.sampleRunStatus              = SampleAutoRunStatus::tooSlowOrHung;
  }

  if ((spawnResult.ExitStatus == 0) && (!spawnResult.MaxExecutionTimePassed)) {
    bool isRenderingEmpty = BABYLON::Samples::ReadScreenshot_IsImageEmpty(sampleName);
    if (isRenderingEmpty) {
      BABYLON_LOG_WARN("ScreenshotAllSamples", "Empty 3D rendering for sample ", sampleName);
      sampleRunInfo.sampleRunStatus              = SampleAutoRunStatus::empty3d;
    }
    else {
      sampleRunInfo.sampleRunStatus = SampleAutoRunStatus::success;
    }
  }
  return sampleRunInfo;
}


void spawnScreenshots(const std::string & exeName, bool flagAsync)
{

#ifdef _WIN32
  BABYLON_LOG_ERROR("spawnScreenshots", " needs fix under windows...");
  return;
#endif

  auto &samplesCollection = SamplesCollection::Instance();
  const auto &allSamples = samplesCollection.AllSamples();
  for (size_t i = 0; i < allSamples.size(); ++i)
  {
    const auto& sampleData = allSamples[i];
    BABYLON_LOG_INFO("spawnScreenshots ", i+1, "/", allSamples.size(), ": ", sampleData.categoryName, "/", sampleData.sampleName);
    auto sampleRunInfo = runOneSample(exeName, sampleData.sampleName, flagAsync);
    samplesCollection.SetSampleRunInfo(sampleData.sampleName, sampleRunInfo);
//    if (i >= 5)
//      break;
  }
  samplesCollection.SaveAllSamplesRunStatuses();
  BABYLON_LOG_INFO("spawnScreenshots", "End, stats:", samplesCollection.GetSampleStatsString().c_str());
}


} // namespace impl
} // namespace BABYLON
