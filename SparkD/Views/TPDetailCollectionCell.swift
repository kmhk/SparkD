//
//  TPDetailCollectionCell.swift
//  SparkD
//
//  Created by com on 6/4/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class TPDetailCollectionCell: UICollectionViewCell {
    
    @IBOutlet weak var containerView: UIView!
    @IBOutlet weak var imageDetail: UIImageView!
    @IBOutlet weak var lblDescription: UILabel!
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        containerView.frame = CGRect(x: 0, y: 0, width: frame.width, height: frame.height - 85)
        containerView.squareGradientView()
        
        imageDetail.frame = CGRect(x: 0, y: 18, width: containerView.frame.width, height: containerView.frame.height - 35)
    }
}
