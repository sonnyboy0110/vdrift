/************************************************************************/
/*                                                                      */
/* This file is part of VDrift.                                         */
/*                                                                      */
/* VDrift is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* VDrift is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with VDrift.  If not, see <http://www.gnu.org/licenses/>.      */
/*                                                                      */
/************************************************************************/

#ifndef _CARSUSPENSION_H
#define _CARSUSPENSION_H

#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"
#include "linearinterp.h"
#include "macros.h"

#include <iosfwd>

class PTree;

struct CarSuspensionInfo
{
	// coilover(const)
	btScalar spring_constant; ///< the suspension spring constant
	btScalar anti_roll; ///< the spring constant for the anti-roll bar
	btScalar bounce; ///< suspension compression damping
	btScalar rebound; ///< suspension decompression damping
	btScalar travel; ///< how far the suspension can travel from the zero-g fully extended position around the hinge arc before wheel travel is stopped
	LinearInterp<btScalar> damper_factors;
	LinearInterp<btScalar> spring_factors;

	// suspension geometry(const)
	btVector3 position; ///< the position of the wheel when the suspension is fully extended (zero g)
	btScalar steering_angle; ///< maximum steering angle in degrees
	btScalar ackermann; ///< /// for ideal ackemann steering_toe = atan(0.5 * steering_axis_length / axes_distance)
	btScalar camber; ///< camber angle in degrees. sign convention depends on the side
	btScalar caster; ///< caster angle in degrees. sign convention depends on the side
	btScalar toe; ///< toe angle in degrees. sign convention depends on the side

	btScalar inv_mass; ///< 1 / unsprung mass

	CarSuspensionInfo(); ///< default constructor makes an S2000-like car
};

class CarSuspension
{
public:
	CarSuspension();

	virtual ~CarSuspension() {}

	const btScalar & GetAntiRoll() const {return info.anti_roll;}

	const btScalar & GetMaxSteeringAngle() const {return info.steering_angle;}

	/// wheel orientation relative to car
	const btQuaternion & GetWheelOrientation() const {return orientation;}

	/// wheel position relative to car
	const btVector3 & GetWheelPosition() const {return position;}

	/// displacement: fraction of suspension travel
	virtual btVector3 GetWheelPosition(btScalar displacement) = 0;

	/// force acting onto wheel
	const btScalar & GetWheelForce() const {return wheel_force;}

	/// suspension force acting onto car body
	const btScalar & GetForce() const {return force;}

	/// wheel overtravel
	const btScalar & GetOvertravel() const {return overtravel;}

	/// wheel displacement
	const btScalar & GetDisplacement() const {return displacement;}

	/// displacement fraction: 0.0 fully extended, 1.0 fully compressed
	btScalar GetDisplacementFraction() const {return displacement / info.travel;}

	btScalar GetDisplacement(btScalar force) const;

	/// steering: -1.0 is maximum right lock and 1.0 is maximum left lock
	virtual void SetSteering(btScalar value);

	/// override current displacement value
	void SetDisplacement(btScalar value);

	/// update displacement, simulate wheel rebound to limit negative delta
	void UpdateDisplacement(btScalar displacement_delta, btScalar dt);

	/// compute suspension and wheel contact forces
	void UpdateForces(btScalar roll_delta, btScalar dt);

	template <class Stream>
	void DebugPrint(Stream & out) const
	{
		out << "---Suspension---" << "\n";
		out << "Displacement: " << displacement << "\n";
		out << "Spring Force: " << spring_force << "\n";
		out << "Damping Force: " << damp_force << "\n";
		out << "Steering angle: " << steering_angle * btScalar(180 / M_PI) << "\n";
	}

	template <class Serializer>
	bool Serialize(Serializer & s)
	{
		_SERIALIZE_(s, steering_angle);
		_SERIALIZE_(s, displacement);
		_SERIALIZE_(s, last_displacement);
		_SERIALIZE_(s, force);
		return true;
	}

	static bool Load(
		const PTree & cfg_wheel,
		btScalar wheel_mass,
		CarSuspension *& suspension,
		std::ostream & error);

protected:
	CarSuspensionInfo info;

	// suspension
	btQuaternion orientation_ext;
	btVector3 steering_axis;
	btQuaternion orientation;
	btVector3 position;
	btScalar steering_angle;
	btScalar spring_force;
	btScalar damp_force;
	btScalar force;

	// wheel
	btScalar overtravel;
	btScalar displacement;
	btScalar last_displacement;
	btScalar wheel_force;
	btScalar wheel_contact;

	void Init(const CarSuspensionInfo & info);
};

#endif
