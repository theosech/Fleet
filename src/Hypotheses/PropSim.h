#pragma once

#include "Datum.h"
#include "Errors.h"

enum PropertyValue
{
        allTrue, allFalse, mixed
};

PropertyValue _get_property_value(const int num_properties_true, const int data_size) {
    PropertyValue value;
    if (num_properties_true == data_size) {
        value = PropertyValue::allTrue;
    } else if (num_properties_true == 0) {
        value = PropertyValue::allFalse;
    } else {
        value = PropertyValue::mixed;
    }
    return value;
}
template<typename input_t, typename output_t, typename prop_output_t, typename _datum_t=defaultdatum_t<input_t, output_t>, typename _data_t=std::vector<_datum_t> >
class Property {
	
public:

	std::string name;
	std::function<prop_output_t(input_t, output_t)> fn;
        double all_true_prior;
        double all_false_prior;
        double mixed_prior;

	Property(std::string property_name, std::function<prop_output_t(input_t, output_t)> property_fn) {
		name = property_name;
		fn = property_fn;
	}

	void print() {
		COUT "name: " << name ENDL;
	}

        double get_joint_prior(PropertyValue task_to_solve_value, PropertyValue candidate_program_value) const {
            return get_prior(task_to_solve_value) * get_prior(candidate_program_value);
        }

        double get_prior(PropertyValue value) const {
            double prior_prob;
            if (value == PropertyValue::allTrue) {    
                    prior_prob = all_true_prior;
            } else if (value == PropertyValue::allFalse) {
                    prior_prob = all_false_prior;
            } else if (value == PropertyValue::mixed) {
                    prior_prob = mixed_prior;
            } else {
                    throw YouShouldNotBeHereError();
            }
            return prior_prob;
        }

        void set_prior(const std::vector<_data_t>& training_set) {

            int all_true_counts = 0;
            int all_false_counts = 0;
            int mixed_counts = 0;

            for (auto& data : training_set) {
                int num_true = 0;
                for (const auto& d : data) {
                    const bool prop_value = apply(d.input, d.output);
                    num_true += prop_value;
                }          
                const PropertyValue prop_value = _get_property_value(num_true, data.size());
                switch(prop_value) {
                    case PropertyValue::allTrue: 
                        all_true_counts += 1;
                        break;
                    case PropertyValue::allFalse: 
                        all_false_counts += 1;
                        break;
                    case PropertyValue::mixed: 
                        mixed_counts += 1;
                        break;
                    // case default : 
                    //    throw YouShouldNotBeHereError("Property value must be one of: allTrue, allFalse, mixed ");
                }
            } 

            all_true_prior = (double)all_true_counts / training_set.size();
            all_false_prior = (double)all_false_counts / training_set.size();
            mixed_prior = (double)mixed_counts / training_set.size();

            COUT "property: " << name ENDL;
            COUT "all_true_prior " << all_true_prior ENDL;
            COUT "all_false_prior " << all_false_prior ENDL;
            COUT "mixed_prior " << mixed_prior ENDLL;
            return;
        }

	prop_output_t apply(const input_t& i, const output_t& o) const {
		return fn(i, o);
	}	
};

/**
 * @class PropSim
 * @author theosech
 * @date 09/06/2021
 * @file PropSim.h
 * @brief A propSim object stores all the parameters relating to how the property similarity score is calculated.
 * Additionally holds the properties with which to calculate the property similarity score
 */
template<typename _datum_t, typename _data_t=std::vector<_datum_t>>
class PropSim {

public:	
	// We'll define a vector of pairs of inputs and outputs
	// this may be used externally to define what data is
	typedef _datum_t datum_t;
	typedef _data_t  data_t; 

	double compute_propsim_score(const data_t& data) {
		return 0.0;
	}

	PropSim() {}
};
