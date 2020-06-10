//
//  ReportTableViewCell.swift
//  SparkD
//
//  Created by com on 6/9/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class ReportTableViewCell: UITableViewCell {

    @IBOutlet weak var containerView: UIView!
    @IBOutlet weak var lblResult: UILabel!
    @IBOutlet weak var lblName: UILabel!
    @IBOutlet weak var lblState: UILabel!
    @IBOutlet weak var lblDate: UILabel!
    
    var gradientLayer: CALayer?
    
    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        lblResult.layer.borderWidth = 1
        lblResult.layer.borderColor = UIColor.systemBlue.cgColor
        lblResult.layer.cornerRadius = 4
        lblResult.backgroundColor = UIColor.lightGray
        lblResult.clipsToBounds = true
        
        if gradientLayer == nil {
            gradientLayer = CALayer()
            gradientLayer!.frame = CGRect(x: 10, y: 10, width: bounds.width - 20, height: bounds.height - 20)
            layer.insertSublayer(gradientLayer!, at: 0)
            
            gradientLayer!.backgroundColor = UIColor.white.cgColor
            gradientLayer!.shadowOffset = CGSize(width: 0, height: 2)
            gradientLayer!.shadowColor = UIColor.gray.cgColor
            gradientLayer!.cornerRadius = 8
            gradientLayer!.shadowOpacity = 0.5
            gradientLayer!.masksToBounds = false
        }
    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }

}
