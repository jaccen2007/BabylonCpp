#ifndef BABYLON_GIZMOS_POSITION_GIZMO_H
#define BABYLON_GIZMOS_POSITION_GIZMO_H

#include <babylon/babylon_global.h>
#include <babylon/gizmos/gizmo.h>

namespace BABYLON {

/**
 * @brief Gizmo that enables dragging a mesh along 3 axis.
 */
class BABYLON_SHARED_EXPORT PositionGizmo : public Gizmo {

public:
  /**
   * @brief Creates a PositionGizmo.
   * @param gizmoLayer The utility layer the gizmo will be added to
   */
  PositionGizmo(UtilityLayerRenderer* gizmoLayer);
  ~PositionGizmo() override;

  /**
   * @brief Disposes of the gizmo.
   */
  void dispose(bool doNotRecurse               = false,
               bool disposeMaterialAndTextures = false) override;

protected:
  void set_attachedMesh(AbstractMesh* const& mesh) override;

  void set_updateGizmoRotationToMatchAttachedMesh(bool value);

  bool get_updateGizmoRotationToMatchAttachedMesh() const;

public:
  Property<PositionGizmo, bool> updateGizmoRotationToMatchAttachedMesh;

private:
  unique_ptr_t<AxisDragGizmo> _xDrag;
  unique_ptr_t<AxisDragGizmo> _yDrag;
  unique_ptr_t<AxisDragGizmo> _zDrag;

}; // end of class PositionGizmo

} // end of namespace BABYLON

#endif // end of BABYLON_GIZMOS_POSITION_GIZMO_H