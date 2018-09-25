#include <babylon/math/isize.h>

namespace BABYLON {

ISize::ISize() : width{0}, height{0}
{
}

ISize::ISize(int iWidth, int iHeight) : width{iWidth}, height{iHeight}
{
}

ISize::ISize(const ISize& other) : width{other.width}, height{other.height}
{
}

ISize::ISize(ISize&& other)
    : width{std::move(other.width)}, height{std::move(other.height)}
{
}

ISize& ISize::operator=(const ISize& other)
{
  if (&other != this) {
    width  = other.width;
    height = other.height;
  }

  return *this;
}

ISize& ISize::operator=(ISize&& other)
{
  if (&other != this) {
    width  = std::move(other.width);
    height = std::move(other.height);
  }

  return *this;
}

ISize::~ISize()
{
}

std::ostream& operator<<(std::ostream& os, const ISize& isize)
{
  os << "{\"Width\":" << isize.width << ",\"Height\":" << isize.height << "}";
  return os;
}

bool operator==(const ISize& lhs, const ISize& rhs)
{
  return (lhs.width == rhs.width) && (lhs.height == rhs.height);
}

bool operator!=(const ISize& lhs, const ISize& rhs)
{
  return !(lhs == rhs);
}

ISize ISize::operator+(const ISize& other) const
{
  return ISize{width + other.width, height + other.height};
}

ISize& ISize::operator+=(const ISize& other)
{
  width += other.width;
  height += other.height;

  return *this;
}

ISize ISize::operator-(const ISize& other) const
{
  return ISize{width - other.width, height - other.height};
}

ISize& ISize::operator-=(const ISize& other)
{
  width -= other.width;
  height -= other.height;

  return *this;
}

ISize ISize::operator*(float scale) const
{
  return ISize{static_cast<int>(static_cast<float>(width) * scale),
               static_cast<int>(static_cast<float>(height) * scale)};
}

ISize& ISize::operator*=(float scale)
{
  width  = static_cast<int>(static_cast<float>(width) * scale);
  height = static_cast<int>(static_cast<float>(height) * scale);

  return *this;
}

ISize ISize::operator/(float scale) const
{
  return operator*(1.f / scale);
}

ISize& ISize::operator/=(float scale)
{
  return operator*=(1.f / scale);
}

} // end of namespace BABYLON
