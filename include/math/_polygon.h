

#ifndef OXLIB_POLYGON_IMP_H
#define OXLIB_POLYGON_IMP_H

#include <type_traits>
#include <vector>
#include <numeric>
#include <cmath>

namespace ox {
    template <typename T>
    concept signed_type = std::is_floating_point_v<T> || std::is_signed_v<T>;

    template <signed_type P>
    struct polygon {
        std::vector<std::pair<P, P>> points;

        constexpr static double distance(std::pair<P, P> p1, std::pair<P, P> p2) {
            P dx = p1.first - p2.first;
            P dy = p1.second - p2.second;
            return std::sqrt(dx * dx + dy * dy);
        }

        [[nodiscard]] constexpr double area() const {
            double res{};
            long size = long(points.size());
            for (long i = 0; i < size; ++i) {
                long j = (i + 1) % size;
                res += (points[i].second + points[j].second) * (points[i].first - points[j].first);
            }
            return std::abs(res / 2);
        }

        [[nodiscard]] constexpr double perimeter() const {
            double res{};
            long size = long(points.size());
            for (long i = 0; i < size; ++i) {
                long j = (i + 1) % size;
                res += distance(points[i], points[j]);
            }
            return std::abs(res);
        }

        [[nodiscard]] constexpr P flat_boundary_count() const {
            double res{};
            long size = long(points.size());
            for (long i = 0; i < size; ++i) {
                long j = (i + 1) % size;
                res += std::abs(points[i].first - points[j].first);
                res += std::abs(points[i].second - points[j].second);
            }
            return res;
        }
    };

} // namespace ox

#endif // OXLIB_POLYGON_IMP_H
