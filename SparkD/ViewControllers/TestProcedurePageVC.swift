//
//  TestProcedurePageVC.swift
//  SparkD
//
//  Created by com on 6/4/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

class TestProcedurePageVC: UIViewController {
    
    var curPage: Int = 0

    @IBOutlet weak var btnSkip: UIButton!
    @IBOutlet weak var btnNext: UIButton!
    
    @IBOutlet weak var pageControl: UIPageControl!
    @IBOutlet weak var collectionView: UICollectionView!
    
    let txtDescipt: [String] = ["Get the Unisampler Device:\nThe UniSampler Device ontains a collection tube (left) and a blood collector with cap (right).",
                                "Add Buffer:\nAdd 5 drops of sample buffer from bottle into the collection tube of Unisampler Device.",
                                "Use a lancet to draw finger-pick blood",
                                "Gently touch the tip of blood collector to blood droplet.\nEnssure that the Unisampler is completely filled with blood as shown in top market.",
                                "Fully insert the blood from the blood collector to the collection tube and push firmly to close tightly.",
                                "Shake the Unisampler and repeat 3~4 times to completely take out blood from blood collector into the buffer with complete mixing.",
                                "Remove the cap of the Unisampler gently exposing the sample dropper",
                                "Invert the Unisampler device and gently squeeze 3 drop of pre-mix bloood into the sample well marked (S) in the test cassette.",
                                "Wait for 15 minutes (start timer in app) and then Scan the cassette 3 times as instructed by app."]
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
        btnSkip.roundSolidButton(title: "Skip")
        btnNext.roundGradientButton(title: "Next")
        
        let layout = UICollectionViewFlowLayout()
        layout.scrollDirection = .horizontal
        collectionView.collectionViewLayout = layout
        
        collectionView.dataSource = self
        collectionView.delegate = self
        collectionView.isPagingEnabled = true
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: String(format: "Step %d", curPage + 1))
        navigationItem.titleView = titleView
        
        scrollToPage(curPage)
    }
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

    @IBAction func btnSkipTapped(_ sender: Any) {
        if let vc = AddTimerController.addTimer() {
            present(vc, animated: true, completion: nil)
        } else {
            let vc = UIAlertController(title: nil, message: "You can add new timer in 2 mins again", preferredStyle: .alert)
            vc.addAction(UIAlertAction(title: "OK", style: .cancel, handler: nil))
            present(vc, animated: true, completion: nil)
        }
    }
    
    @IBAction func btnNextTapped(_ sender: Any) {
        if curPage < 8 {
            curPage += 1
            scrollToPage(curPage)
        } else {
            btnSkipTapped(sender)
        }
    }
    
    // private method
    
    func scrollToPage(_ page: Int) {
        let indexpath = collectionView.indexPathForItem(at: CGPoint(x: page * Int(collectionView.frame.size.width), y: 0))
        collectionView.scrollToItem(at: indexpath!, at: .left, animated: false)
        
        pageControl.currentPage = curPage
    }
}


// MARK: -
extension TestProcedurePageVC: UICollectionViewDataSource, UICollectionViewDelegateFlowLayout, UICollectionViewDelegate {
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 9
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "TPDetailCollectionCell", for: indexPath) as! TPDetailCollectionCell
        
        let imageName = String(format: "tutorial_step%d", indexPath.row + 1)
        let image = UIImage(named: imageName)
        cell.imageDetail.image = image        
        
        cell.lblDescription.text = txtDescipt[indexPath.row]
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        return collectionView.frame.size
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, insetForSectionAt section: Int) -> UIEdgeInsets {
        return UIEdgeInsets(top: 0, left: 0, bottom: 0, right: 0)
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, minimumLineSpacingForSectionAt section: Int) -> CGFloat {
        return 0
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, minimumInteritemSpacingForSectionAt section: Int) -> CGFloat {
        return 0
    }
    
    func scrollViewDidScroll(_ scrollView: UIScrollView) {
        curPage = Int(scrollView.contentOffset.x / collectionView.frame.width)
        
        let titleView = navigationController?.navTitleWithImageAndText(titleText: String(format: "Step %d", curPage + 1))
        navigationItem.titleView = titleView
        
        pageControl.currentPage = curPage
    }
}
