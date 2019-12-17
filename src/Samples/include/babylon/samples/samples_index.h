#ifndef BABYLON_SAMPLES_SAMPLES_INDEX_H
#define BABYLON_SAMPLES_SAMPLES_INDEX_H
#include <babylon/babylon_api.h>
#include <babylon/samples/samples_common.h>
#include <memory>
#include <optional>
#include <vector>

namespace BABYLON {
namespace Samples {

struct SampleInfo {
  std::string HeaderFile;
  std::string SourceFile;
  std::string Brief;
  std::vector<std::string> Links;
};

enum class SampleFailureReasonKind {
  segFault,
  processHung,
  empty3d,
  broken3d
};

struct SampleFailureReason
{
  SampleFailureReasonKind Kind;
  std::string Info;

  SampleFailureReason(
    SampleFailureReasonKind kind = SampleFailureReasonKind::segFault, 
    std::string info = "")
    : Kind(kind), Info(info) {}
};

std::string BABYLON_SHARED_EXPORT SampleFailureReason_Str(SampleFailureReasonKind s);

class BABYLON_SHARED_EXPORT SamplesIndex {
private:
  SamplesIndex();

public:
  static SamplesIndex & Instance();
  ~SamplesIndex();

  void RegisterSample(
    const std::string& categoryName, 
    const std::string & sampleName,
    SampleFactoryFunction fn);

  /**
   * @brief Check if the sample is currently known to fail
   * @param sampleName the name of the sample to check
   * @ return an optional string describing why it fails
   */
  std::optional<SampleFailureReason> doesSampleFail(const std::string& sampleName) const;

  /**
   * @brief Returns whether or not the example exists.
   * @param sampleName the name of the sample to check
   * @return whether or not the example exists
   */
  bool sampleExists(const std::string& sampleName) const;

  /**
   * @brief Returns info about the sample, by parsing the source file tree.
   * @param sampleName the name of the sample to check
   * @return SampleInfo struct. May throw if sample was not found.
   */
  SampleInfo getSampleInfo(const std::string& sampleName) const;

  /**
   * @brief Returns a sorted list containing the enabled examples.
   * @return a sorted list containing the enabled examples.
   */
  std::vector<std::string> getSampleNames() const;

  /**
   * @brief Returns a sorted list containing the category names.
   * @return a sorted list containing the category names.
   */
  std::vector<std::string> getCategoryNames() const;

  /**
   * @brief Returns whether or not the category exists.
   * @param categoryNameToSearch the name of the category to check
   * @return whether or not the category exists
   */
  bool categoryExists(const std::string& categoryNameToSearch) const;

  /**
   * @brief Returns a sorted list containing the enabled examples in the
   * specified category.
   * @return a sorted list containing the enabled examples in the specified
   * category.
   */
  std::vector<std::string>
  getSampleNamesInCategory(const std::string& categoryName) const;

  /**
   * @brief Sample factory method
   * @param sampleName the name of the sample to create
   * @param iCanvas the canves to be used
   * @return the renderable scene
   */
  IRenderableScenePtr
  createRenderableScene(const std::string& sampleName, ICanvas* iCanvas) const;

  /**
  * @ brief Output the list of samples to stdout
  **/
  void listSamples();

private:
  void fillSamplesFailures() const;

  // Contains the mapping from category to samples index
  std::unordered_map<std::string, _ISamplesIndex> _samplesIndex;

  // Contains the failing examples.
  mutable std::unordered_map<std::string, SampleFailureReason> _samplesFailures;

}; // end of class SamplesIndex

} // end of namespace Samples
} // end of namespace BABYLON

#endif // end of BABYLON_SAMPLES_SAMPLES_INDEX_H
